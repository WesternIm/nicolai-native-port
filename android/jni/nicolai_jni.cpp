#include <jni.h>
#include <string>
#include "nicolai/engine.hpp"
#include "nicolai/russian_legacy.hpp"
#include "nicolai/russian_stress.hpp"

namespace {
std::string jstr(JNIEnv* env, jstring s) {
    if (!s) return {};
    const char* p = env->GetStringUTFChars(s, nullptr);
    std::string out = p ? p : "";
    if (p) env->ReleaseStringUTFChars(s, p);
    return out;
}

jshortArray pcm_to_java(JNIEnv* env, const nicolai::SynthResult& r) {
    if (r.status != nicolai::SynthStatus::Ok) return env->NewShortArray(0);
    auto arr = env->NewShortArray(static_cast<jsize>(r.pcm.samples.size()));
    if (!r.pcm.samples.empty()) {
        env->SetShortArrayRegion(arr, 0, static_cast<jsize>(r.pcm.samples.size()),
                                 reinterpret_cast<const jshort*>(r.pcm.samples.data()));
    }
    return arr;
}
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_nicolai_NativeTts_probeVoice(JNIEnv* env, jobject, jstring path) {
    try {
        auto db = nicolai::VoiceDb::load(jstr(env, path));
        const auto& m = db.metadata();
        std::string out = "Nicolai=" + std::string(m.has_nicolai_name ? "yes" : "no") +
                          ";rate=" + std::to_string(m.sample_rate_hint) +
                          ";psola=" + std::string(m.has_tempo_psola_marker ? "yes" : "no");
        return env->NewStringUTF(out.c_str());
    } catch (const std::exception& e) {
        return env->NewStringUTF(e.what());
    }
}

extern "C" JNIEXPORT jshortArray JNICALL
Java_com_example_nicolai_NativeTts_synthesizePcm16(JNIEnv* env, jobject, jstring dbPath, jstring text) {
    try {
        nicolai::Engine engine(nicolai::VoiceDb::load(jstr(env, dbPath)));
        return pcm_to_java(env, engine.synthesize(jstr(env, text)));
    } catch (...) {
        return env->NewShortArray(0);
    }
}

extern "C" JNIEXPORT jshortArray JNICALL
Java_com_example_nicolai_NativeTts_synthesizePcm16WithStressDictionary(
    JNIEnv* env, jobject, jstring dbPath, jstring excRusPath, jstring text) {
    try {
        const auto exc_path = jstr(env, excRusPath);
        auto stress = nicolai::load_exc_rus_cp1251(exc_path);
        nicolai::Engine engine(nicolai::VoiceDb::load(jstr(env, dbPath)), std::move(stress));
        return pcm_to_java(env, engine.synthesize(jstr(env, text)));
    } catch (...) {
        return env->NewShortArray(0);
    }
}

extern "C" JNIEXPORT jshortArray JNICALL
Java_com_example_nicolai_NativeTts_synthesizePcm16WithLegacyDictionaries(
    JNIEnv* env, jobject, jstring dbPath, jstring excRusPath, jstring abbRusPath, jstring text) {
    try {
        const auto exc_path = jstr(env, excRusPath);
        const auto abb_path = jstr(env, abbRusPath);
        auto stress = nicolai::load_exc_rus_cp1251(exc_path);
        auto exceptions = nicolai::load_exc_rus_replacements_cp1251(exc_path);
        auto abbreviations = nicolai::load_abb_rus_cp1251(abb_path);
        nicolai::Engine engine(nicolai::VoiceDb::load(jstr(env, dbPath)),
                               std::move(stress),
                               std::move(abbreviations),
                               std::move(exceptions));
        return pcm_to_java(env, engine.synthesize(jstr(env, text)));
    } catch (...) {
        return env->NewShortArray(0);
    }
}
