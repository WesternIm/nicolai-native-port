#include "nicolai/legacy_window_m36.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>

namespace nicolai {
namespace {
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr int kMinWindow = 20;
constexpr int kMaxWindow = 400;

std::int16_t word(int value) {
    const auto u = static_cast<std::uint16_t>(value);
    return static_cast<std::int16_t>(
        u <= 32767 ? static_cast<int>(u) : static_cast<int>(u) - 65536);
}

std::vector<int> anchors_impl() {
    std::vector<int> out;
    int value = kMinWindow;
    while (value < kMaxWindow) {
        out.push_back(value);
        // 0x1010a0bc..0x1010a0cb: fild current; fmul 1.23; _ftol.
        int next = static_cast<int>(static_cast<double>(value) * 1.23);
        if (next <= value) ++next;
        value = next;
    }
    if (out.empty() || out.back() != kMaxWindow) out.push_back(kMaxWindow);
    return out;
}

std::vector<std::int16_t> descending_cosine_core(int length) {
    std::vector<std::int16_t> out;
    if (length <= 0) return out;
    out.reserve(static_cast<std::size_t>(length));
    // 0x1010a238 stores pi/previous_length to a 32-bit float before the loop.
    const float step = static_cast<float>(kPi / static_cast<double>(length));
    for (int i = 0; i < length; ++i) {
        const double angle = static_cast<double>(i) * static_cast<double>(step);
        const double scaled =
            (std::cos(angle) + 1.0) * 0.5 * 32767.0;
        // Imported _ftol is used by the original positive-domain construction.
        const int q = static_cast<int>(scaled);
        out.push_back(static_cast<std::int16_t>(std::clamp(q, 0, 32767)));
    }
    return out;
}

struct WindowCache {
    std::vector<int> anchors;
    std::vector<std::int16_t> packed;
    std::vector<int> offset; // requested length 20+i -> packed WORD offset
};

WindowCache build_cache() {
    WindowCache c;
    c.anchors = anchors_impl();
    c.offset.assign(static_cast<std::size_t>(kMaxWindow - kMinWindow + 1), -1);

    std::map<int, std::vector<std::int16_t>> anchor_window;
    int previous = kMinWindow; // callback arg4 points at the 20..400 range.
    for (int current : c.anchors) {
        const int gap = current - previous;
        const int left_pad = gap / 2;
        const int right_pad = current - left_pad - previous;
        std::vector<std::int16_t> w;
        w.reserve(static_cast<std::size_t>(current));
        w.insert(w.end(), static_cast<std::size_t>(left_pad),
            static_cast<std::int16_t>(32767));
        const auto core = descending_cosine_core(previous);
        w.insert(w.end(), core.begin(), core.end());
        w.insert(w.end(), static_cast<std::size_t>(right_pad),
            static_cast<std::int16_t>(0));
        anchor_window.emplace(current, w);
        previous = current;
    }

    for (int anchor : c.anchors) {
        c.offset[static_cast<std::size_t>(anchor - kMinWindow)] =
            static_cast<int>(c.packed.size());
        const auto& w = anchor_window.at(anchor);
        c.packed.insert(c.packed.end(), w.begin(), w.end());
    }

    // 0x1010a37f..0x1010a3a4 fills every non-anchor table entry with a pointer
    // into the single packed WORD area rather than allocating a separate window.
    for (std::size_t i = 0; i + 1 < c.anchors.size(); ++i) {
        const int lower = c.anchors[i];
        const int upper = c.anchors[i + 1];
        const int boundary =
            c.offset[static_cast<std::size_t>(lower - kMinWindow)] + lower;
        const int zero_slots = upper - lower - 1;
        for (int requested = lower + 1; requested < upper; ++requested) {
            const int distance = requested - lower;
            c.offset[static_cast<std::size_t>(requested - kMinWindow)] =
                boundary + zero_slots / 2 - distance + 1;
        }
    }
    return c;
}

LegacyWindowM36 direct_from_cache(const WindowCache& c, int requested_length) {
    LegacyWindowM36 out;
    out.requested_length = requested_length;
    if (requested_length < kMinWindow || requested_length >= kMaxWindow)
        return out;
    const int slot = requested_length - kMinWindow;
    if (slot < 0 || slot >= static_cast<int>(c.offset.size())) return out;
    const int start = c.offset[static_cast<std::size_t>(slot)];
    if (start < 0 || start + requested_length > static_cast<int>(c.packed.size()))
        return out;

    out.q15.assign(c.packed.begin() + start,
        c.packed.begin() + start + requested_length);
    out.valid = true;
    out.source_cache_length = requested_length;
    const auto exact = std::find(c.anchors.begin(), c.anchors.end(), requested_length);
    if (exact != c.anchors.end()) {
        out.lower_anchor = requested_length;
        out.upper_anchor = requested_length;
        out.exact_anchor = true;
        return out;
    }
    for (std::size_t i = 0; i + 1 < c.anchors.size(); ++i) {
        if (c.anchors[i] < requested_length && requested_length < c.anchors[i + 1]) {
            out.lower_anchor = c.anchors[i];
            out.upper_anchor = c.anchors[i + 1];
            break;
        }
    }
    return out;
}
}

std::vector<int> legacy_window_m36_anchors() {
    return anchors_impl();
}

LegacyWindowM36 legacy_window_m36_direct(int requested_length) {
    return direct_from_cache(build_cache(), requested_length);
}

LegacyWindowM36 legacy_window_m36_lookup(int requested_length) {
    LegacyWindowM36 out;
    out.requested_length = requested_length;
    if (requested_length <= 0 || requested_length > kMaxWindow) return out;

    const auto cache = build_cache();
    if (requested_length >= kMinWindow && requested_length < kMaxWindow)
        return direct_from_cache(cache, requested_length);

    out.resampled = true;
    if (requested_length < kMinWindow) {
        // 0x10109c64..0x10109cf0. Table header DWORD #1 is 2, so the
        // selected offset slot is factor*requested (after removing that header).
        int factor = 2;
        while (2 + factor * requested_length <= kMinWindow) factor *= 2;
        const int source_slot = factor * requested_length;
        if (source_slot < 0 || source_slot >= static_cast<int>(cache.offset.size()))
            return out;
        int source = cache.offset[static_cast<std::size_t>(source_slot)];
        if (source < 0) return out;
        out.q15.reserve(static_cast<std::size_t>(requested_length));
        for (int i = 0; i < requested_length; ++i) {
            const int at = source + i * factor;
            if (at < 0 || at >= static_cast<int>(cache.packed.size())) return {};
            out.q15.push_back(cache.packed[static_cast<std::size_t>(at)]);
        }
        out.valid = true;
        out.resample_factor = factor;
        out.source_cache_length = kMinWindow + source_slot;
        return out;
    }

    // requested_length == 400 in this guarded M36 domain. 0x10109d04 onward
    // chooses a power-of-two expansion source from the same packed cache.
    int factor = 2;
    while (requested_length / factor + 2 >= kMaxWindow) factor *= 2;
    const int source_slot = requested_length / factor;
    if (source_slot < 0 || source_slot >= static_cast<int>(cache.offset.size()))
        return out;
    const int source_base = cache.offset[static_cast<std::size_t>(source_slot)];
    if (source_base < 0) return out;

    out.q15.assign(static_cast<std::size_t>(requested_length), 0);
    int source_index = 0;
    for (int output_index = 0; output_index < requested_length;
         output_index += factor) {
        const int first = source_base + source_index;
        if (first < 0 || first >= static_cast<int>(cache.packed.size())) return {};
        out.q15[static_cast<std::size_t>(output_index)] =
            cache.packed[static_cast<std::size_t>(first)];
        if (factor <= 1) continue;
        for (int j = 1; j < factor && output_index + j < requested_length; ++j) {
            const int cur = source_base + source_index;
            const int next = cur + 1;
            if (cur < 0 || next >= static_cast<int>(cache.packed.size())) return {};
            const int sum = static_cast<int>(cache.packed[static_cast<std::size_t>(cur)]) +
                static_cast<int>(cache.packed[static_cast<std::size_t>(next)]);
            const int value = sum * j / factor; // signed IDIV truncates to zero
            out.q15[static_cast<std::size_t>(output_index + j)] = word(value);
            ++source_index;
        }
    }
    out.valid = true;
    out.resample_factor = factor;
    out.source_cache_length = kMinWindow + source_slot;
    return out;
}

} // namespace nicolai
