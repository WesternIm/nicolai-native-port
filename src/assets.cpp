#include "nicolai/assets.hpp"
#include "nicolai/address_space.hpp"
#include "nicolai/voice_catalog.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace nicolai {
namespace {
std::uint64_t next_init_target_after(const std::vector<std::uint8_t>& bytes,
                                     const EdatLayout& layout,
                                     std::uint64_t after_file_offset) {
    const auto range = make_edat_range_descriptor(layout);
    std::uint64_t best = layout.segment0_end;
    for (const auto& ref : scan_serialized_static_refs(bytes, layout)) {
        const auto rr = resolve_static_ref({range}, StaticDuration::Initialization, ref);
        if (!rr.valid) continue;
        if (rr.file_offset > after_file_offset && rr.file_offset < best) best = rr.file_offset;
    }
    return best;
}

std::string ascii_runs(const std::vector<std::uint8_t>& b, std::uint64_t o, std::size_t n) {
    if (o >= b.size()) return {};
    const auto end = std::min<std::uint64_t>(b.size(), o + n);
    std::string best, cur;
    for (std::uint64_t i=o;i<end;++i) {
        const auto c=b[static_cast<std::size_t>(i)];
        if (c>=0x20 && c<=0x7e) cur.push_back(static_cast<char>(c));
        else { if(cur.size()>best.size()) best=cur; cur.clear(); }
    }
    if(cur.size()>best.size()) best=cur;
    return best;
}
}

VoiceAssetMap locate_nicolai_voice_assets(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const std::vector<TaggedRef>& /*legacy_refs*/) {

    VoiceAssetMap m;
    if (!layout.valid) { m.error="invalid_edat_layout"; return m; }

    // M10 correction: serialized targets are logical offsets. Reuse the
    // initialization-duration legacy file objects recovered from the voice
    // graph instead of treating target values as raw file offsets.
    const auto init_objects = scan_legacy_file_objects(bytes, layout, StaticDuration::Initialization);
    const auto* seg = find_legacy_file_object(init_objects, "nbr16aci.seg");
    const auto* ana = find_legacy_file_object(init_objects, "nbr16aci.ana");
    if (!seg || !ana) { m.error="nicolai_acoustic_file_objects_not_found"; return m; }

    const auto range = make_edat_range_descriptor(layout);
    const auto seg_data = resolve_static_ref({range}, StaticDuration::Initialization,
                                             kEdatTaggedRefMagic, seg->data_logical);
    const auto ana_data = resolve_static_ref({range}, StaticDuration::Initialization,
                                             kEdatTaggedRefMagic, ana->data_logical);
    if (!seg_data.valid || !ana_data.valid || seg_data.file_offset >= ana_data.file_offset) {
        m.error="invalid_nicolai_seg_ana_chain"; return m;
    }

    const auto voice_end = next_init_target_after(bytes, layout, ana_data.file_offset);
    if (voice_end <= ana_data.file_offset || voice_end > layout.segment0_end) {
        m.error="compressed_voice_end_not_found"; return m;
    }

    // M10: the 86,308-byte variable-record directory is nbr16aci.seg;
    // the 6,132,372-byte cmp16 bitstream is nbr16aci.ana. Legacy field
    // names are retained for compatibility with older probes.
    m.valid=true;
    m.ana_path_offset=ana->file_offset;
    m.ana_descriptor_offset=ana->file_offset;
    m.analysis_object_offset=seg->file_offset;
    m.seg_object_offset=seg->file_offset;
    m.analysis_data_offset=seg_data.file_offset;
    m.analysis_data_end=ana_data.file_offset;
    m.analysis_data_size=m.analysis_data_end-m.analysis_data_offset;
    m.compressed_voice_offset=ana_data.file_offset;
    m.compressed_voice_end=voice_end;
    m.compressed_voice_size=m.compressed_voice_end-m.compressed_voice_offset;
    m.next_object_offset=voice_end;
    m.next_object_ascii=ascii_runs(bytes,voice_end,0x200);
    return m;
}

} // namespace nicolai
