#include "nicolai/voice_db.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>

namespace nicolai {
namespace {

std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) return 0;
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o + 1]) << 8) |
           (static_cast<std::uint32_t>(b[o + 2]) << 16) |
           (static_cast<std::uint32_t>(b[o + 3]) << 24);
}

std::size_t find_ascii(const std::vector<std::uint8_t>& b, const std::string& needle) {
    const auto it = std::search(b.begin(), b.end(), needle.begin(), needle.end());
    return it == b.end() ? std::string::npos : static_cast<std::size_t>(it - b.begin());
}

std::vector<std::string> extract_interesting_ascii(const std::vector<std::uint8_t>& b) {
    std::vector<std::string> out;
    std::string cur;
    auto flush = [&]() {
        if (cur.size() >= 8) {
            std::string low = cur;
            std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            const bool interesting =
                low.find("nicolai") != std::string::npos ||
                low.find("russian") != std::string::npos ||
                low.find("russkij") != std::string::npos ||
                low.find("psola") != std::string::npos ||
                low.find("diphone") != std::string::npos ||
                low.find(".dsc") != std::string::npos ||
                low.find(".rgl") != std::string::npos ||
                low.find(".axm") != std::string::npos ||
                low.find(".seg") != std::string::npos ||
                low.find(".ana") != std::string::npos;
            if (interesting && std::find(out.begin(), out.end(), cur) == out.end()) {
                out.push_back(cur);
            }
        }
        cur.clear();
    };

    for (std::uint8_t v : b) {
        if (v >= 0x20 && v <= 0x7e) {
            cur.push_back(static_cast<char>(v));
            if (cur.size() > 512) flush();
        } else {
            flush();
        }
        if (out.size() >= 32) break;
    }
    flush();
    return out;
}

} // namespace

VoiceDb VoiceDb::load(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open voice database: " + path.string());

    VoiceDb db;
    f.seekg(0, std::ios::end);
    const auto size = f.tellg();
    if (size <= 0) throw std::runtime_error("voice database is empty");
    f.seekg(0, std::ios::beg);

    db.bytes_.resize(static_cast<std::size_t>(size));
    f.read(reinterpret_cast<char*>(db.bytes_.data()), static_cast<std::streamsize>(db.bytes_.size()));
    if (!f) throw std::runtime_error("failed reading voice database");

    auto& m = db.metadata_;
    m.size_bytes = db.bytes_.size();
    m.header_word0 = le32(db.bytes_, 0);
    m.header_word1 = le32(db.bytes_, 4);
    m.tagged_magic = le32(db.bytes_, 8);

    const auto name = find_ascii(db.bytes_, "Nicolai");
    m.has_nicolai_name = name != std::string::npos;
    m.nicolai_name_offset = m.has_nicolai_name ? name : 0;

    const auto psola = find_ascii(db.bytes_, "tempo-psola");
    const auto russ = find_ascii(db.bytes_, "Syc.Russkij.DataDir");
    m.has_tempo_psola_marker = psola != std::string::npos;
    m.has_russian_frontend_marker = russ != std::string::npos;

    // This is a hint only. The supplied database is named nicolai16.dat and its
    // embedded build paths contain "16aci"; the original installer registers
    // Nicolai as 16000 Hz. Synthesis code must still validate its own output format.
    if (find_ascii(db.bytes_, "16aci") != std::string::npos) m.sample_rate_hint = 16000;

    m.edat = parse_edat_layout(db.bytes_);
    m.tagged_refs = scan_tagged_refs(db.bytes_, m.edat);
    m.mode_records = scan_nicolai_mode_records(db.bytes_, m.edat);
    m.resource_paths = find_nicolai_resource_paths(db.bytes_);
    if (!m.mode_records.empty()) m.sample_rate_hint = static_cast<int>(m.mode_records.front().sample_rate);

    m.evidence_strings = extract_interesting_ascii(db.bytes_);
    return db;
}

} // namespace nicolai
