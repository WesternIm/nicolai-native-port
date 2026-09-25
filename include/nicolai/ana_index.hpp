#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct AnaUnitRecord {
    std::size_t index = 0;
    std::size_t table_offset = 0;       // offset relative to analysis_data
    std::int32_t compressed_start = 0;  // offset relative to compressed voice pool
    std::int32_t signed_end = 0;        // sign is preserved: meaning not yet assigned
    std::uint32_t compressed_end = 0;   // absolute value of signed_end
    std::size_t record_size = 0;
    std::vector<std::int16_t> metadata;
    bool terminal = false;
};

struct AnaUnitIndex {
    bool valid = false;
    std::string error;
    std::vector<AnaUnitRecord> records;
    std::uint32_t covered_bytes = 0;
    std::uint32_t payload_bytes = 0;
    std::uint32_t trailing_payload_bytes = 0;
    std::size_t trailing_analysis_bytes = 0;
};

// Legacy M3 parser retained for compatibility. M10 established that this
// variable-length directory is the contents of nbr16aci.seg, while the
// following cmp16 bitstream is nbr16aci.ana. New code should prefer
// DiphoneCatalog, which obtains exact SEG record boundaries from AXM instead
// of scanning heuristically. The robust invariant used here is:
//   record[n+1].start == abs(record[n].signed_end)
// Records are variable-sized; the next record is discovered by locating that
// boundary value in the bounded forward window rather than assuming a fixed
// layout.  No semantic meaning is assigned to the sign bit yet.
AnaUnitIndex parse_ana_unit_index(
    const std::vector<std::uint8_t>& analysis_data,
    std::uint32_t compressed_payload_size,
    std::size_t max_record_scan = 1024);

std::vector<std::uint8_t> extract_compressed_unit(
    const std::vector<std::uint8_t>& compressed_payload,
    const AnaUnitRecord& unit);

} // namespace nicolai
