#define NICOLAI_CAPTURE_TEST
#include "../tools/nicolai_m36_runtime_capture.cpp"
#include <cassert>
#include <cstring>

int main() {
    // Synthetic in-process memory tests ONLY; not original-runtime evidence.
    std::vector<BYTE> state(0x100, 0), descriptor(0x60, 0);
    std::int16_t record[5]{3, 2, 100, -4, 7};
    auto put32 = [](std::vector<BYTE>& bytes, int offset, std::uint32_t value) {
        std::memcpy(bytes.data() + offset, &value, sizeof(value));
    };
    auto pointer = [](const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); };
    put32(state, 0x74, pointer(record));
    put32(state, 0x48, 321);
    std::ostringstream out;
    json_runtime_state(out, GetCurrentProcess(), pointer(state.data()));
    assert(out.str().find("\"cursors\":[321,0,0,0,0,0]") != std::string::npos);
    assert(out.str().find("\"slot_74\":[3,2,100,-4,7]") != std::string::npos);
    assert(out.str().find("\"slot_78\":null") != std::string::npos);
    std::int16_t nodes = 3, duration[2]{2048, 1024}, pitch[2]{100, 120};
    std::int32_t positions[3]{0, 100, 220};
    BYTE cross = 2, flag14 = 0;
    put32(descriptor, 0x10, pointer(&cross));
    put32(descriptor, 0x14, pointer(&flag14));
    put32(descriptor, 0x18, pointer(&nodes));
    put32(descriptor, 0x24, pointer(duration));
    put32(descriptor, 0x28, pointer(pitch));
    put32(descriptor, 0x30, pointer(positions));
    out.str(""); out.clear();
    json_runtime_descriptor(out, GetCurrentProcess(), pointer(descriptor.data()));
    assert(out.str().find("\"cross_flag\":2") != std::string::npos);
    assert(out.str().find("\"source_position\":[0,100,220]") != std::string::npos);
    nodes = 1001;
    bool rejected = false;
    try { json_runtime_descriptor(out, GetCurrentProcess(), pointer(descriptor.data())); }
    catch (const std::runtime_error&) { rejected = true; }
    assert(rejected);
    out.str(""); out.clear();
    json_runtime_descriptor(out, GetCurrentProcess(), 0);
    assert(out.str() == "null");
    rejected = false;
    try { json_runtime_state(out, GetCurrentProcess(), 0); }
    catch (const std::runtime_error&) { rejected = true; }
    assert(rejected);
    std::cout << "synthetic route snapshot contracts passed\n";
}
