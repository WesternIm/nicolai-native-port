#pragma once
#include <cstdint>
#include <vector>

namespace nicolai {

struct LegacyWindowM36 {
    bool valid = false;
    int requested_length = 0;
    int lower_anchor = 0;
    int upper_anchor = 0;
    bool exact_anchor = false;
    bool resampled = false;
    int resample_factor = 1;
    int source_cache_length = 0;
    std::vector<std::int16_t> q15;
};

// Static reconstruction of the direct cached domain used by 0x10109be0.
// The original cache is built by 0x1010a020 for lengths 20..400, while the
// direct pointer lookup covers requested lengths [20, 400).
LegacyWindowM36 legacy_window_m36_direct(int requested_length);

// Recovered guarded runtime domain 1..400. Requests below 20 use the original
// power-of-two decimation branch. A request of exactly 400 uses the original
// >=max expansion branch (factor 2). Larger lengths remain outside this M36
// contract until the full packed-buffer upper-domain bounds are proven.
LegacyWindowM36 legacy_window_m36_lookup(int requested_length);

std::vector<int> legacy_window_m36_anchors();

} // namespace nicolai
