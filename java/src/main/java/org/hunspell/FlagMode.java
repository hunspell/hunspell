package org.hunspell;

/**
 * Affix flag encoding modes mirroring the C++ Hunspell {@code FLAG_MODE}
 * enumeration in {@code hashmgr.cxx}.
 */
enum FlagMode {
    /** Single-character ASCII flags ({@code FLAG_CHAR}). */
    CHAR,
    /** Two-character ASCII flags ({@code FLAG long}). */
    LONG,
    /** Decimal numbers separated by commas ({@code FLAG num}). */
    NUM,
    /** UTF-8 single-codepoint flags ({@code FLAG UTF-8}). */
    UTF8
}
