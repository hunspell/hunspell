package org.hunspell;

import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.Set;

/**
 * Helpers for decoding Hunspell affix flag vectors into integer flag IDs.
 *
 * <p>Mirrors {@code HashMgr::decode_flags} from {@code hashmgr.cxx}: an empty
 * vector decodes to no flags, and the parsing strategy depends on the active
 * {@link FlagMode}.</p>
 */
final class Flags {

    private Flags() {
    }

    /** Decode a raw flag vector string into ordered, deduplicated flag IDs. */
    static int[] decode(String raw, FlagMode mode) {
        if (raw == null || raw.isEmpty()) {
            return new int[0];
        }
        Set<Integer> ordered = new LinkedHashSet<>();
        switch (mode) {
            case CHAR -> {
                for (int i = 0; i < raw.length(); i++) {
                    ordered.add(raw.charAt(i) & 0xFFFF);
                }
            }
            case LONG -> {
                int len = raw.length();
                for (int i = 0; i + 1 < len; i += 2) {
                    int hi = raw.charAt(i) & 0xFF;
                    int lo = raw.charAt(i + 1) & 0xFF;
                    ordered.add((hi << 8) | lo);
                }
                if ((len & 1) == 1) {
                    // Match C++ tolerant behavior: trailing odd char becomes its own flag.
                    ordered.add(raw.charAt(len - 1) & 0xFFFF);
                }
            }
            case NUM -> {
                for (String token : raw.split(",")) {
                    String trimmed = token.strip();
                    if (trimmed.isEmpty()) {
                        continue;
                    }
                    try {
                        int value = Integer.parseInt(trimmed);
                        if (value >= 0 && value <= 0xFFFF) {
                            ordered.add(value);
                        }
                    } catch (NumberFormatException ignored) {
                        // skip malformed flag id, mirroring lenient C++ parser
                    }
                }
            }
            case UTF8 -> {
                int i = 0;
                while (i < raw.length()) {
                    int cp = raw.codePointAt(i);
                    ordered.add(cp);
                    i += Character.charCount(cp);
                }
            }
        }
        int[] out = new int[ordered.size()];
        int idx = 0;
        for (int v : ordered) {
            out[idx++] = v;
        }
        return out;
    }

    /** Decode a single affix flag identifier (the flag attached to a {@code PFX}/{@code SFX} rule). */
    static int decodeSingle(String token, FlagMode mode) {
        int[] decoded = decode(token, mode);
        if (decoded.length == 0) {
            throw new HunspellParseException("Empty affix flag token");
        }
        return decoded[0];
    }

    static boolean contains(int[] flags, int needle) {
        for (int f : flags) {
            if (f == needle) {
                return true;
            }
        }
        return false;
    }

    static int[] union(int[] a, int[] b) {
        if (a.length == 0) {
            return b.clone();
        }
        if (b.length == 0) {
            return a.clone();
        }
        Set<Integer> ordered = new LinkedHashSet<>(a.length + b.length);
        for (int v : a) {
            ordered.add(v);
        }
        for (int v : b) {
            ordered.add(v);
        }
        int[] out = new int[ordered.size()];
        int idx = 0;
        for (int v : ordered) {
            out[idx++] = v;
        }
        return out;
    }

    static String formatForDebug(int[] flags) {
        return Arrays.toString(flags);
    }
}
