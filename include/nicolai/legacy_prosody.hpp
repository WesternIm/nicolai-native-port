#pragma once

#include "nicolai/diphone_catalog.hpp"
#include "nicolai/hybrid_psola.hpp"
#include "nicolai/russian_frontend.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace nicolai {

struct LegacyPhoneDurationProfile {
    bool valid = false;
    std::string error;
    std::unordered_map<std::string,int> milliseconds;
};

struct LegacyWordProsodyProfile {
    bool valid = false;
    std::string error;
    std::vector<float> values; // exact 35-float static image of rusvox wordstr.par
};

// M26: exact embedded rusvox\data\physical.int image recovered from the
// Nicolai voice database. The PC loader allocates 14 records of 42 signed
// bytes. mtsyc32.dll indexes them by the physical-prosody class array and uses
// bytes 18..41 to author the three [t%d] pitch-percent fields later copied to
// runtime +0x190/+0x194/+0x198.
struct LegacyPhysicalProsodyRecord {
    std::array<std::int8_t,42> values{};
};

struct LegacyPhysicalProsodyProfile {
    bool valid = false;
    std::string error;
    std::vector<LegacyPhysicalProsodyRecord> records;
};

LegacyPhoneDurationProfile parse_legacy_russian_phone_durations(
    const std::vector<std::uint8_t>& database_bytes,
    const EdatLayout& layout);

LegacyWordProsodyProfile parse_legacy_russian_wordstr(
    const std::vector<std::uint8_t>& database_bytes,
    const EdatLayout& layout);

LegacyPhysicalProsodyProfile parse_legacy_russian_physical(
    const std::vector<std::uint8_t>& database_bytes,
    const EdatLayout& layout);

// Diagnostic normalized view of the 7-node triplet groups inside one
// physical.int record. position is normalized to [0,1]; base_slot selects
// legacy bytes 18,19,20. The table layout is recovered exactly, while the
// complete PC branch choosing nodes/positions is still being ported.
std::array<double,3> legacy_physical_pitch_triplet(
    const LegacyPhysicalProsodyRecord& record,
    double position,
    int base_slot = 18);

// Exact arithmetic helpers recovered at mtsyc32.dll 0x10215d30/0x10215d90.
int legacy_physical_interp_add_pc(int additive, int start, int end, int position, int count);
int legacy_physical_interp_pc(int start, int end, int position, int count);

// M28: exact empty-marker branch recovered from mtsyc32.dll:0x1021570c..
// The legacy orthographic pronunciation notation uses a second '<' stress
// marker; when that marker is immediately followed by '>' the old writer
// takes one of three proven paths. The first non-final empty marker uses lane
// selector k0, subsequent non-final empty markers use k2 with the exact
// 0x10215d30 interpolation, and the final global empty marker uses k4 from
// record[20]. k5 is not referenced by the t-writer at all.
std::array<double,3> legacy_physical_empty_marker_triplet_pc(
    const LegacyPhysicalProsodyRecord& record,
    bool first_empty_marker,
    bool last_global_marker,
    int base_slot,
    int marker_position,
    int marker_count);

// PC chooses byte 18 vs 19 by comparing the number of marker-bearing words
// in the current span against wordstr[24] (2.0 in Nicolai).
int legacy_physical_base_slot_pc(int marker_word_count, double threshold = 2.0);

// M29: exact ordinary single-'<' authoring path recovered from
// mtsyc32.dll:0x10215ae7..0x10215bc1.  The k1/k3 guard in this Nicolai
// build is backed by a stack local that is initialized to zero and has no
// reachable writes or aliases before the selector, so the live path always
// uses k1 (22/29/36).  Interpolation position/count are the stressed vowel
// ordinal and total vowel count in the current span, not a synthetic marker
// ordinal.  wordstr[33] contributes the exact -0.5*value integer bias used by
// the PC writer before record[18]/record[19].
std::array<double,3> legacy_physical_single_marker_triplet_pc(
    const LegacyPhysicalProsodyRecord& record,
    int marker_word_count,
    int vowel_position,
    int vowel_count,
    double wordstr_24 = 2.0,
    double wordstr_33 = 20.0);

// M31: [l%d]/[e%d] share the same physical.int authoring pass as [t%d].
// physical bytes 3..7 and 8..12 are the two 5-byte length profiles, selected
// by marker-bearing-word count versus wordstr[24]. Bytes 13..17 are the
// 5-byte energy profile. These helpers reproduce the branch arithmetic at
// mtsyc32.dll 0x1021524a..0x10215617. marker_ordinal is 1-based.
int legacy_physical_length_percent_pc(
    const LegacyPhysicalProsodyRecord& record,
    int marker_word_count,
    int marker_ordinal,
    bool marker_is_greater,
    bool has_double_left_from_marker,
    bool has_triple_left_in_word,
    double wordstr_24 = 2.0);

int legacy_physical_energy_percent_pc(
    const LegacyPhysicalProsodyRecord& record,
    int marker_word_count,
    int marker_ordinal,
    bool marker_is_greater,
    bool has_double_left_from_marker,
    bool has_triple_left_in_word);

// M31: exact lower-level dynamic span-split primitive recovered from
// 0x10216c70. The PC counts an item when it has '<' or when the per-item
// legacy skip flag is clear, then picks the nearest empty-code separator to
// the midpoint. Minor '/' split uses a space candidate at effective count>=5;
// hard '(/)' split uses '_' at effective count>6. Exposing the primitive keeps
// the portable diagnostic honest while the producer of the raw ' '/ '_'
// separator lattice is still being named.
struct LegacyAuthoringSplitItemM31 {
    char raw_separator = 0;
    bool code_empty = true;
    bool annotated_has_left = false;
    bool legacy_skip_count = false;
};

struct LegacyAuthoringSplitDecisionM31 {
    bool split = false;
    std::size_t index = 0;
    int effective_count = 0;
};

LegacyAuthoringSplitDecisionM31 legacy_authoring_split_decision_m31(
    const std::vector<LegacyAuthoringSplitItemM31>& items,
    std::size_t first, std::size_t last,
    char candidate_separator, int threshold, bool strict_greater);


struct LegacyTimingPolicy {
    // M20 PC-reference conformance pass. These are intentionally separate from
    // duration.par: the old Russian prosody layer applies additional word- and
    // boundary-level timing coefficients after the phone-duration lookup.
    double phone_duration_scale = 1.11;
    double utterance_boundary_scale = 1.00;
    double word_boundary_scale = 0.45;
    double comma_boundary_scale = 1.40;
    double weak_punctuation_boundary_scale = 1.20;
    double strong_punctuation_boundary_scale = 1.60;

    // M21: relative utterance-position contour recovered from wordstr.par
    // indices 7/12/17.  The raw legacy contour is normalized over the words
    // in the utterance so this stage changes local pacing without reintroducing
    // a corpus-wide duration bias already corrected in M20.
    bool enable_wordstr_position_contour = true;
    // M23 retune after restoring the missing pitch layer. M21 needed a
    // stronger 0.25 timing contour partly to compensate for pitch-preserving
    // synthesis; with PC-like F0 regulation the golden corpus prefers 0.075.
    double wordstr_contour_strength = 0.075;

    // M23: redistribute each already-calibrated diphone target between its
    // left/right phone sides using the SEG split and a global phone-duration
    // support lattice.  The per-diphone total target is renormalized back to
    // the M22 value, so this primarily changes transition timing/waveform
    // shape rather than corpus-wide duration.  0 preserves M22 exactly.
    double phone_side_strength = 0.0;
    double phone_side_ratio_limit = 1.8;

    // Reference-derived, phone-class timing calibration.  These remain 1.0
    // unless explicitly enabled/tuned; unlike phrase-specific exceptions they
    // generalize by diphone transition class.  M23 uses them only if a golden
    // corpus fit survives regularized validation.
    double cc_duration_scale = 1.0;
    double cv_duration_scale = 1.0;
    double vc_duration_scale = 1.0;
    double vv_duration_scale = 1.0;

    // M23 pitch conformance. The PC engine stores a default Russian base pitch
    // of 83 in runtime +0x81264 (pitch.par parser target). Function 0x101a2250
    // then performs the exact legacy transform
    //   p' = base + (p-base) * wordstr[28]
    // where Nicolai wordstr[28] is 0.95. The earlier upstream pitch-contour
    // generator is not yet fully named; until M24, portable synthesis rebuilds
    // its input from SEG pitch by retaining 5/19 of the SEG deviation from the
    // 83-Hz centre. The subsequent 0.95 step is exact, giving an effective
    // residual of 0.25. This single corpus-wide coefficient was selected on the
    // 22-phrase PC oracle, not by phrase-specific exceptions.
    double legacy_pitch_strength = 1.0;
    double legacy_pitch_base_hz = 83.0;
    double legacy_pitch_source_residual = 5.0 / 19.0;

    // M24's smooth three-point utterance declination remains the production
    // backbone in M25. A pure replacement by reconstructed sparse anchors was
    // tested and rejected because it regressed the 22-phrase golden corpus;
    // the recovered sparse-anchor machinery is therefore introduced only as
    // a weak correction until the upstream PC authoring rules are named.
    double pitch_declination_strength = 1.0;
    double pitch_declination_start = 1.04;
    double pitch_declination_mid = 0.94;
    double pitch_declination_end = 0.78;
    double pitch_stressed_vowel_boost = 1.0;

    // M25: sparse PC-style pitch-anchor lattice. Reverse engineering of
    // mtsyc32.dll shows that source feature records are 0x20 bytes and may
    // carry three signed pitch percentages at +0x10/+0x14/+0x18. 10213150
    // copies them to runtime +0x190/+0x194/+0x198 under the explicit-pitch
    // feature flag; 10227630 searches forward for the next anchor and the
    // central prosody pass linearly bridges sparse anchors. The exact lexical
    // authoring rule that fills every source triplet is not fully recovered,
    // so M25 reconstructs only the placement/value layer with general
    // phonetic cues (phrase edge + stressed-vowel anchors), never phrases.
    // Percentages are interpreted around pitch.par base=83 and then pass
    // through the recovered wordstr[28] centering transform.
    double pitch_anchor_strength = 0.05;            // weak M25 correction
    double pitch_anchor_start_percent = 4.0;
    double pitch_anchor_end_percent = -18.0;
    double pitch_anchor_stress_lift_percent = 12.0;
    double pitch_anchor_long_phrase_start_boost_percent = 4.0;

    // M26: weak blend of the recovered physical.int pitch table. Zero is an
    // exact M25 fallback. The table itself and interpolation are exact; until
    // every lexical/morphological branch of 0x10215de0 is ported, the portable
    // class selector intentionally uses only sentence-level cues that are
    // already preserved by the frontend.
    double physical_pitch_strength = 0.0;
    // M27: the final t-node of 0x10214f40 is fully recovered (record[20]
    // plus record[25/32/39]). It can therefore be blended independently from
    // the still-incomplete interior physical.int node selector.
    double physical_terminal_pitch_strength = 0.016;
    // M28: independently blend the recovered *non-terminal* standard
    // orthographic empty-stress-marker path (<...<>). Exact terminal <> keeps
    // the already validated M27 terminal strength, so experiments here do not
    // weaken/confound the terminal correction.
    double physical_empty_marker_pitch_strength = 0.0;
    // M29: independently blend the now-recovered ordinary single-< stress
    // marker path (k1 + exact vowel-position interpolation).  Zero preserves
    // M28 audio bit-for-bit; corpus experiments decide whether this can be
    // enabled without regressing the PC oracle.
    double physical_single_marker_pitch_strength = 0.002;

    // M31 experimental projection of the exact physical.int [l%d] and [e%d]
    // authoring profiles onto the portable diphone renderer. [l] is a signed
    // percent duration correction in the PC runtime (base*(1+l/100)); [e] is
    // an absolute 0..100 energy target. Zero preserves M30 exactly.
    double physical_length_strength = 0.0;
    double physical_energy_strength = 0.0;
    // Keep the M27 last-phone terminal approximation available while M28
    // measures the narrower exact marker path. It can be set to zero without
    // affecting the recovered empty-marker anchors.
    // M26 reverse-engineering controls.  The PC engine stores one physical
    // class byte per word in state+0x130 and uses it to select one of the
    // 14 physical.int rows.  Class 0 is the neutral all-zero row.  The
    // override is diagnostic only; -1 uses the punctuation-derived selector.
    int physical_terminal_class_override = -1;
    int physical_internal_class = 0;
    // M27 classifier controls. The recovered PC classifier uses class 3 for
    // WH-questions and distinguishes classes 7/8 for other questions using a
    // morphology/POS byte lattice not yet present in the portable frontend.
    // 8 is the conservative fallback; diagnostics may override it.
    int physical_nonwh_question_class = 8;
    bool physical_use_punctuation_classes = true;
};

// M27 exact/proven portion of the physical prosody classifier. Returns one
// physical.int row class per frontend word. Unsupported proprietary morphology
// distinctions use the policy fallback rather than phrase-specific guesses.
std::vector<int> legacy_physical_class_lattice_m27(
    const RussianFrontendResult& frontend,
    const LegacyTimingPolicy& policy = {});

// M30: authoring-span state recovered around mtsyc32.dll 0x10214f40.
// The legacy [t%d] writer does not select physical.int independently per word:
// it scans from the current word to the next state+0x12c == "(/)" boundary,
// counts marker-bearing words and vowels across that whole span, and selects
// the physical.int row from the span-ending state+0x130 class.  The portable
// frontend has no raw state+0x12c lattice, so definite sentence-level
// punctuation is used as the conservative observable equivalent; no dynamic
// long-span split is invented here.
struct LegacyAuthoringSpanState {
    std::size_t first_word = 0;
    std::size_t last_word = 0;
    int physical_class = 0;
    int marker_word_count = 0;
    int vowel_count = 0;
};

std::vector<LegacyAuthoringSpanState> legacy_authoring_spans_m30(
    const RussianFrontendResult& frontend,
    const LegacyTimingPolicy& policy = {});

// M23 portable reconstruction of the PC pitch-centering stage. The exact
// downstream PC arithmetic uses wordstr[28]; the source-residual parameter
// stands in for the not-yet-fully-recovered upstream contour generator.
double legacy_pitch_target_f0_m23(
    double source_f0_hz,
    const LegacyWordProsodyProfile* wordstr,
    const LegacyTimingPolicy& policy = {});

// M22: exact CP866 symbol identities recovered from mtsyc32.dll.  These are
// not generic "feature bytes": they are literal Russian phoneme symbols used
// by the legacy previous/next-symbol walkers.
enum class LegacyRecoveredSymbol { Other, I, J, R, U };

LegacyRecoveredSymbol legacy_recovered_symbol(const std::string& phone);

struct LegacyRecoveredRuleHits {
    int r_adjacent = 0;
    int j_to_i = 0;
    int u_j_u_left_u = 0;
    int u_j_u_center_j = 0;
    int u_j_u_right_u = 0;
};

LegacyRecoveredRuleHits analyze_legacy_recovered_neighbor_rules(
    const std::vector<std::string>& phones);

struct LegacyTimedDiphone {
    std::string label;
    double source_ms = 0.0;
    double target_ms = 0.0;
    double duration_scale = 1.0;
    double left_duration_scale = 1.0;
    double right_duration_scale = 1.0;
};

struct DiphoneChainLegacyResult {
    bool valid = false;
    std::string error;
    Pcm16Mono pcm;
    std::vector<LegacyTimedDiphone> timings;
};

// M20 duration/prosody conformance renderer. Nicolai's embedded rusvox
// duration.par supplies phone targets, while the original PC prosody layer
// additionally retimes word boundaries. M20 preserves punctuation kind in the
// frontend and applies a PC-reference-calibrated timing policy instead of
// treating every # boundary as a full raw diphone.
DiphoneChainLegacyResult synthesize_diphone_chain_legacy_duration(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    const LegacyPhoneDurationProfile& durations,
    const LegacyWordProsodyProfile* wordstr = nullptr,
    double pitch_scale = 1.0,
    int sample_rate = 16000,
    const std::vector<FrontendBoundary>& boundaries = {},
    const LegacyTimingPolicy& timing_policy = {},
    const LegacyPhysicalProsodyProfile* physical = nullptr,
    const RussianFrontendResult* frontend = nullptr);

// M21 PC-output framing. The supplied Windows oracle corpus consistently
// contains an additional terminal quiet flush. The recovered wordstr tail
// provides a 150-ms timing parameter; the PC corpus matches a two-quantum
// terminal flush closely, so this remains data-backed but explicitly marked
// as a conformance hypothesis until the exact legacy branch is named.
std::size_t legacy_pc_terminal_silence_samples(
    const LegacyWordProsodyProfile& wordstr, int sample_rate = 16000);

void append_legacy_pc_terminal_silence(
    Pcm16Mono& pcm, const LegacyWordProsodyProfile& wordstr, int sample_rate = 16000);

} // namespace nicolai
