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

std::map<int, std::vector<std::int16_t>> anchor_windows(
    const std::vector<int>& anchors) {
    std::map<int, std::vector<std::int16_t>> out;
    int previous = kMinWindow; // callback arg4 points at the 20..400 range.
    for (int current : anchors) {
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
        out.emplace(current, std::move(w));
        previous = current;
    }
    return out;
}
}

std::vector<int> legacy_window_m36_anchors() {
    return anchors_impl();
}

LegacyWindowM36 legacy_window_m36_direct(int requested_length) {
    LegacyWindowM36 out;
    out.requested_length = requested_length;
    if (requested_length < kMinWindow || requested_length >= kMaxWindow)
        return out;

    const auto anchors = anchors_impl();
    const auto windows = anchor_windows(anchors);

    const auto exact = windows.find(requested_length);
    if (exact != windows.end()) {
        out.valid = true;
        out.lower_anchor = requested_length;
        out.upper_anchor = requested_length;
        out.exact_anchor = true;
        out.q15 = exact->second;
        return out;
    }

    for (std::size_t i = 0; i + 1 < anchors.size(); ++i) {
        const int lower = anchors[i];
        const int upper = anchors[i + 1];
        if (!(lower < requested_length && requested_length < upper)) continue;

        const int zero_slots = upper - lower - 1;
        const int distance = requested_length - lower;
        // 0x1010a37f..0x1010a3a4 stores an offset into the single packed WORD
        // area. Re-express that pointer relative to W_lower || W_upper.
        const int start = lower + zero_slots / 2 - distance + 1;
        const auto& a = windows.at(lower);
        const auto& b = windows.at(upper);
        std::vector<std::int16_t> packed;
        packed.reserve(a.size() + b.size());
        packed.insert(packed.end(), a.begin(), a.end());
        packed.insert(packed.end(), b.begin(), b.end());
        if (start < 0 ||
            start + requested_length > static_cast<int>(packed.size()))
            return out;

        out.q15.assign(packed.begin() + start,
            packed.begin() + start + requested_length);
        out.valid = true;
        out.lower_anchor = lower;
        out.upper_anchor = upper;
        out.exact_anchor = false;
        return out;
    }
    return out;
}

} // namespace nicolai
