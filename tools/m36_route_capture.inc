// Included inside the feature debugger's anonymous namespace. No disk patches.
constexpr std::uintptr_t kRouteEntry = 0x107c20;
struct RouteCall {
    std::uint64_t sequence;
    std::uintptr_t state, previous, current;
    unsigned stages = 0;
};

void json_runtime_state(std::ostream& out, HANDLE process, std::uintptr_t ptr) {
    out << "{\"markers\":";
    json_array(out, read_vector<std::int16_t>(process, ptr + 0x24, 6));
    out << ",\"fields\":";
    json_array(out, read_vector<std::int32_t>(process, ptr + 0x30, 5));
    out << ",\"cursors\":";
    json_array(out, read_vector<std::int32_t>(process, ptr + 0x48, 6));
    out << ",\"end_cursor\":" << read_value<std::int32_t>(process, ptr + 0x80);
    out << ",\"carry\":" << read_value<std::int16_t>(process, ptr + 0x7c);
    for (auto slot : {0x74, 0x78}) {
        out << ",\"slot_" << (slot == 0x74 ? "74" : "78") << "\":";
        const auto record = read_value<std::uint32_t>(process, ptr + slot);
        if (record) json_array(out, read_vector<std::int16_t>(process, record, 5));
        else out << "null";
    }
    out << '}';
}

void json_runtime_descriptor(std::ostream& out, HANDLE process, std::uintptr_t ptr) {
    if (!ptr) { out << "null"; return; }
    const auto nodes = read_value<std::int16_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x18));
    if (nodes < 2 || nodes > kMaxNodes)
        throw std::runtime_error("runtime descriptor node count outside guarded domain");
    const auto flag = [&](int offset) {
        return read_value<BYTE>(process, read_value<std::uint32_t>(process, ptr + offset));
    };
    out << "{\"cross_flag\":" << static_cast<int>(flag(0x10))
        << ",\"descriptor_flag14\":" << static_cast<int>(flag(0x14))
        << ",\"node_count\":" << nodes << ",\"source_position\":";
    json_array(out, read_vector<std::int32_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x30), nodes));
    out << ",\"duration_q11\":";
    json_array(out, read_vector<std::int16_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x24), nodes - 1));
    out << ",\"pitch_q11\":";
    json_array(out, read_vector<std::int16_t>(process,
        read_value<std::uint32_t>(process, ptr + 0x28), nodes - 1));
    out << ",\"pcm_address\":" << read_value<std::uint32_t>(process, ptr + 0x4c) << '}';
}

int run_route_capture(DWORD pid, const std::string& output, const std::string& stop,
                      bool linguistic = false, DWORD parent_pid = 0, bool render_host = false) {
    HANDLE process = nullptr;
    HANDLE owner = nullptr;
    struct OwnerGuard { HANDLE& h; ~OwnerGuard() { if (h) CloseHandle(h); } } owner_guard{owner};
    bool attached = false;
    bool active_event = false;
    bool exited = false;
    bool owner_gone = false;
    bool mutated = false, hooks_ready = false;
    std::uintptr_t base = 0;
    DEBUG_EVENT ev{};
    std::map<std::uintptr_t, BYTE> originals;
    std::map<DWORD, StepState> stepping;
    // Stops occur between instructions; preserve EIP/flags when detaching.
    auto restore = [&] {
        if (exited) return;
        bool failed = false;
        for (const auto& bp : originals) {
            try { write_byte(process, bp.first, bp.second); }
            catch (...) { failed = true; }
        }
        for (auto& step : stepping) {
            HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
                FALSE, step.first);
            if (thread) {
                CONTEXT ctx{}; ctx.ContextFlags = CONTEXT_CONTROL;
                if (GetThreadContext(thread, &ctx)) {
                    ctx.EFlags &= ~static_cast<DWORD>(0x100);
                    if (!SetThreadContext(thread, &ctx)) failed = true;
                } else failed = true;
                CloseHandle(thread);
            } else failed = true;
            resume_threads(step.second.suspended_threads);
        }
        if (failed) throw std::runtime_error("could not restore every breakpoint");
    };
    try {
        if (linguistic) {
            validate_owner_m46(pid, parent_pid, render_host);
            owner = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, parent_pid);
            if (!owner || WaitForSingleObject(owner, 0) != WAIT_TIMEOUT)
                throw std::runtime_error("owned trigger exited before capture");
        }
        if (GetFileAttributesW(std::filesystem::u8path(output).c_str()) != INVALID_FILE_ATTRIBUTES)
            throw std::runtime_error("route capture output must be fresh");
        base = find_module_base(pid, L"mtsyc32.dll");
        if (!base && !render_host) throw std::runtime_error("mtsyc32.dll is not loaded");
        process = OpenProcess(linguistic ? kLinguisticAccessM46 :
            (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION), FALSE, pid);
        if (!process) throw std::runtime_error("OpenProcess failed");
        if (linguistic) {
            FILETIME parent_start{}, target_start{}, a{}, b{}, c{};
            if (!GetProcessTimes(owner, &parent_start, &a, &b, &c) ||
                !GetProcessTimes(process, &target_start, &a, &b, &c) ||
                (render_host ? CompareFileTime(&target_start, &parent_start) != 0 :
                               CompareFileTime(&target_start, &parent_start) < 0))
                throw std::runtime_error("owned-process lifetime changed");
        }
        if (base) {
            // Preserve early fail-closed behavior for an already loaded image.
            validate_remote_image(process, base);
            if (linguistic) validate_file_m46(pid);
        }
        // Instruction starts verified against the pinned 2004 image. Actual
        // writer call sites, not inferred voicing, identify the observed route.
        const std::map<std::uintptr_t, std::string> points = linguistic ?
            std::map<std::uintptr_t, std::string>{
                {0x1a17ed, "before_split"}, {0x1a17f3, "after_split"},
                {0x1a17ff, "after_markers"}, {0x1a1811, "after_authoring"}} :
            std::map<std::uintptr_t, std::string>{
            {kRouteEntry, "entry"}, {0x107d54, "decision"},
            {0x107d8b, "cross"}, {0x107e28, "initial"},
            {0x107ebe, "ordinary"}, {0x108004, "interval_end"},
            {0x10801b, "post_loop"}, {0x108076, "terminal_flush"},
            {0x10808b, "failure_return"}, {0x10820d, "return"}};
        auto arm = [&](const std::filesystem::path& loaded_path) {
            // At attach, or a LOAD_DLL event, ALL target threads are stopped.
            // Validate the entire image/point set before changing any byte.
            validate_remote_image(process, base);
            if (linguistic) {
                if (loaded_path.empty()) validate_file_m46(pid);
                else validate_file_m46(loaded_path);
            }
            std::map<std::uintptr_t, BYTE> verified;
            for (const auto& point : points) {
                const auto byte = read_byte(process, base + point.first);
                if (linguistic && byte != 0x55)
                    throw std::runtime_error("linguistic instruction fingerprint mismatch");
                verified.emplace(base + point.first, byte);
            }
            originals = std::move(verified);
            mutated = true; // Includes a partial write failure: fail closed.
            for (const auto& bp : originals) write_byte(process, bp.first, 0xcc);
            hooks_ready = true;
            if (linguistic) std::cout << "M46_HOOKS_READY pinned_original=1\n" << std::flush;
        };
        std::ofstream out(std::filesystem::u8path(output), std::ios::binary);
        if (!out) throw std::runtime_error("cannot open route output");
        if (!DebugActiveProcess(pid)) throw std::runtime_error("DebugActiveProcess failed");
        attached = true;
        if (!DebugSetProcessKillOnExit(FALSE)) throw std::runtime_error("cannot set detach policy");
        if (base) arm({});
        SetConsoleCtrlHandler(console_handler, TRUE);
        std::map<DWORD, RouteCall> pending;
        std::uint64_t sequence = 0, records = 0, skipped = 0, completed = 0;
        std::cout << "M36_RUNTIME_CAPTURE_READY mode=" << (linguistic ? "linguistics" : "route")
                  << " pid=" << pid << " hooks=" << (hooks_ready ? "ready" : "waiting-for-module")
                  << '\n' << std::flush;
        bool detach_requested = false;
        const auto capture_deadline = GetTickCount64() + 90000;
        ULONGLONG stop_deadline = 0;
        while (true) {
            if (linguistic) {
                owner_gone = WaitForSingleObject(owner, 0) != WAIT_TIMEOUT;
                if (owner_gone || GetTickCount64() >= capture_deadline) g_stop = true;
                if (file_exists(stop) && !stop_deadline) stop_deadline = GetTickCount64() + 3000;
                if (stop_deadline && GetTickCount64() >= stop_deadline) g_stop = true;
            }
            // Restore while a debugger event stops ALL target threads. Restoring
            // into a running process can race a queued INT3 and skip an opcode.
            if (!detach_requested && (g_stop ||
                    (file_exists(stop) && pending.empty() && stepping.empty()))) {
                if (!DebugBreakProcess(process)) throw std::runtime_error("detach break failed");
                detach_requested = true;
            }
            if (!WaitForDebugEvent(&ev, 100)) {
                if (GetLastError() == ERROR_SEM_TIMEOUT) continue;
                throw std::runtime_error("WaitForDebugEvent failed");
            }
            active_event = true;
            // Read the LOAD_DLL file identity before closing its event handle.
            const auto image_path = render_host && ev.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT ?
                debug_image_path_m46(ev.u.LoadDll.hFile) : std::filesystem::path{};
            close_debug_image_file_m46(ev);
            DWORD status = DBG_CONTINUE;
            bool detach_now = false;
            if (render_host && !hooks_ready && !detach_requested &&
                ev.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT &&
                _wcsicmp(image_path.filename().c_str(), L"mtsyc32.dll") == 0) {
                base = reinterpret_cast<std::uintptr_t>(ev.u.LoadDll.lpBaseOfDll);
                arm(image_path);
            } else if (hooks_ready && ev.dwDebugEventCode == UNLOAD_DLL_DEBUG_EVENT &&
                reinterpret_cast<std::uintptr_t>(ev.u.UnloadDll.lpBaseOfDll) == base) {
                // Never restore to unmapped/reused addresses after module unload.
                originals.clear();
                throw std::runtime_error("original module unloaded during capture");
            } else if (ev.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                const auto code = ev.u.Exception.ExceptionRecord.ExceptionCode;
                const auto address = reinterpret_cast<std::uintptr_t>(
                    ev.u.Exception.ExceptionRecord.ExceptionAddress);
                auto bp = originals.find(address);
                if ((code == EXCEPTION_BREAKPOINT && bp != originals.end()) ||
                    (code == EXCEPTION_SINGLE_STEP && stepping.count(ev.dwThreadId))) {
                    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT,
                        FALSE, ev.dwThreadId);
                    if (!thread) throw std::runtime_error("OpenThread failed");
                    struct ThreadGuard { HANDLE h; ~ThreadGuard() { CloseHandle(h); } } guard{thread};
                    CONTEXT ctx{}; ctx.ContextFlags = CONTEXT_FULL;
                    if (!GetThreadContext(thread, &ctx)) throw std::runtime_error("GetThreadContext failed");
                    if (code == EXCEPTION_SINGLE_STEP) {
                        auto& step = stepping.at(ev.dwThreadId);
                        write_byte(process, step.breakpoint_address, 0xcc);
                        ctx.EFlags &= ~static_cast<DWORD>(0x100);
                        if (!SetThreadContext(thread, &ctx)) throw std::runtime_error("SetThreadContext step failed");
                        resume_threads(step.suspended_threads);
                        stepping.erase(ev.dwThreadId);
                    } else {
                        // Rewind first, so even capture errors can safely detach.
                        write_byte(process, address, bp->second);
                        ctx.Eip = static_cast<DWORD>(address);
                        if (!SetThreadContext(thread, &ctx)) throw std::runtime_error("SetThreadContext rewind failed");
                        const auto rva = address - base;
                        const auto& name = points.at(rva);
                        try {
                            if (linguistic && name == "before_split") {
                                if (pending.count(ev.dwThreadId)) throw std::runtime_error("nested linguistic call");
                                pending[ev.dwThreadId] = RouteCall{++sequence, ctx.Ebp, 0, 0};
                            } else if (rva == kRouteEntry) {
                                if (pending.count(ev.dwThreadId)) throw std::runtime_error("nested route call");
                                RouteCall call{++sequence,
                                    read_value<std::uint32_t>(process, ctx.Esp + 16),
                                    read_value<std::uint32_t>(process, ctx.Esp + 8),
                                    read_value<std::uint32_t>(process, ctx.Esp + 12)};
                                pending[ev.dwThreadId] = call;
                            }
                            auto it = pending.find(ev.dwThreadId);
                            if (it != pending.end()) {
                                // Stage the whole row to prevent partial JSON on read failure.
                                std::ostringstream row;
                                row << "{\"schema\":\"" << (linguistic ? "nicolai-m46-linguistic-v1" : "nicolai-m36-route-v1")
                                    << "\",\"call\":"
                                    << it->second.sequence << ",\"thread_id\":" << ev.dwThreadId
                                    << ",\"event\":\"" << name << "\",\"state\":";
                                if (linguistic) {
                                    if (ctx.Ebp != it->second.state) throw std::runtime_error("linguistic state changed");
                                    const unsigned stage = name == "before_split" ? 0 :
                                        name == "after_split" ? 1 : name == "after_markers" ? 2 : 3;
                                    if (it->second.stages != stage)
                                        throw std::runtime_error("linguistic stage out of order");
                                    json_linguistic_state_m46(row, process, it->second.state, name == "after_authoring");
                                } else json_runtime_state(row, process, it->second.state);
                                if (!linguistic && rva == kRouteEntry) {
                                    const auto context = read_value<std::uint32_t>(process, ctx.Esp + 4);
                                    row << ",\"flush_gate\":" << read_value<std::int32_t>(process, context + 0x7c)
                                        << ",\"previous\":";
                                    json_runtime_descriptor(row, process, it->second.previous);
                                    row << ",\"current\":";
                                    json_runtime_descriptor(row, process, it->second.current);
                                } else if (name == "decision") {
                                    row << ",\"interval_index\":" << ctx.Esi
                                        << ",\"cross_pending\":" << read_value<std::int16_t>(process, ctx.Esp + 0x48)
                                        << ",\"already_started\":" << read_value<std::int32_t>(process, ctx.Esp + 0x14)
                                        << ",\"flush_gate_word\":" << read_value<std::int16_t>(process, ctx.Esp + 0x1c)
                                        << ",\"step\":";
                                    auto step = read_vector<std::int16_t>(process, ctx.Esp + 0x30, 5);
                                    step[0] = static_cast<std::int16_t>(ctx.Esi);
                                    json_array(row, step);
                                } else if (name == "return" || name == "failure_return") {
                                    row << ",\"return_value\":" << ctx.Eax;
                                }
                                row << "}\n";
                                out << row.str() << std::flush;
                                if (!out) throw std::runtime_error("route write failed");
                                ++records;
                                if (linguistic && ++it->second.stages == 4) ++completed;
                            } else if (linguistic) {
                                throw std::runtime_error("linguistic stage without its entry");
                            }
                        } catch (const std::exception& error) {
                            if (linguistic) std::cerr << "M46 rejected stage=" << name
                                << " reason=" << error.what() << '\n' << std::flush;
                            ++skipped;
                            pending.erase(ev.dwThreadId);
                        }
                        if (name == "return" || name == "failure_return" ||
                            (linguistic && name == "after_authoring")) pending.erase(ev.dwThreadId);
                        StepState step;
                        step.breakpoint_address = address;
                        step.suspended_threads = suspend_other_threads(pid, ev.dwThreadId);
                        stepping[ev.dwThreadId] = std::move(step);
                        ctx.EFlags |= 0x100;
                        if (!SetThreadContext(thread, &ctx)) throw std::runtime_error("SetThreadContext trace failed");
                    }
                } else if (code == EXCEPTION_BREAKPOINT && detach_requested) {
                    restore();
                    detach_now = true;
                } else if (code != EXCEPTION_BREAKPOINT) status = DBG_EXCEPTION_NOT_HANDLED;
            } else if (ev.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
                if (linguistic && pending.count(ev.dwThreadId)) ++skipped;
                pending.erase(ev.dwThreadId);
                auto it = stepping.find(ev.dwThreadId);
                if (it != stepping.end()) { resume_threads(it->second.suspended_threads); stepping.erase(it); }
            } else if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) exited = true;
            if (!ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, status))
                throw std::runtime_error("ContinueDebugEvent failed");
            active_event = false;
            if (exited || detach_now) break;
        }
        if (!exited && !DebugActiveProcessStop(pid)) throw std::runtime_error("detach failed");
        attached = false;
        // A GUI cancellation kills the render parent without running its RAII.
        // The worker watches that retained parent handle and cleans up only its
        // verified server child, after restoring/detaching the breakpoints.
        if (linguistic && owner_gone && !exited) TerminateProcess(process, ERROR_CANCELLED);
        CloseHandle(process);
        std::cout << "{\"records\":" << records << ",\"skipped\":" << skipped
                  << ",\"completed_calls\":" << completed << "}\n";
        if (linguistic && !hooks_ready) std::cerr << "original module not loaded in owned capture target\n";
        return records && !skipped && pending.empty() &&
            (!linguistic || (!owner_gone && !exited && hooks_ready &&
                            completed && records == completed * 4)) ? 0 : 3;
    } catch (const std::exception& e) {
        if (linguistic && attached && mutated && !exited) {
            // Exceptional detach cannot safely restore into a running target.
            // Fail closed on THIS verified child; never leave live INT3 bytes.
            TerminateProcess(process, ERROR_CANCELLED);
        }
        if (attached) { try { restore(); } catch (...) {} }
        if (active_event) ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE);
        if (attached && !exited) DebugActiveProcessStop(pid);
        if (process) CloseHandle(process);
        std::cerr << e.what() << '\n';
        return 1;
    }
}
