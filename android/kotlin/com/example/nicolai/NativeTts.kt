package com.example.nicolai

class NativeTts {
    external fun probeVoice(path: String): String
    external fun synthesizePcm16(dbPath: String, text: String): ShortArray
    external fun synthesizePcm16WithStressDictionary(
        dbPath: String,
        excRusPath: String,
        text: String
    ): ShortArray
    external fun synthesizePcm16WithLegacyDictionaries(
        dbPath: String,
        excRusPath: String,
        abbRusPath: String,
        text: String
    ): ShortArray

    companion object {
        init { System.loadLibrary("nicolai_native") }
    }
}
