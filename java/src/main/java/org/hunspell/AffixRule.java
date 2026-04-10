package org.hunspell;

import java.util.regex.Pattern;

/**
 * Single {@code PFX} or {@code SFX} entry mirroring the structure of
 * {@code PfxEntry}/{@code SfxEntry} in {@code affentry.cxx}.
 *
 * <p>Each rule carries the flag that identifies it, the strip/append parts,
 * a compiled condition pattern, and the optional continuation flag IDs that
 * the resulting form inherits (used to chain affix application like the C++
 * {@code contclass}).</p>
 */
final class AffixRule {
    private final int flag;
    private final boolean prefix;
    private final boolean crossProduct;
    private final String strip;
    private final String append;
    private final Pattern condition;
    private final int[] contFlags;

    AffixRule(int flag, boolean prefix, boolean crossProduct, String strip, String append,
              String condition, int[] contFlags) {
        this.flag = flag;
        this.prefix = prefix;
        this.crossProduct = crossProduct;
        this.strip = strip;
        this.append = append;
        this.condition = compileCondition(condition, prefix);
        this.contFlags = contFlags;
    }

    int flag() {
        return flag;
    }

    boolean prefix() {
        return prefix;
    }

    boolean crossProduct() {
        return crossProduct;
    }

    String strip() {
        return strip;
    }

    String append() {
        return append;
    }

    int[] contFlags() {
        return contFlags;
    }

    boolean hasContClass() {
        return contFlags.length > 0;
    }

    /** Apply this rule to a stem, returning the generated form or {@code null}. */
    String apply(String stem) {
        if (!matchesStem(stem)) {
            return null;
        }
        if (prefix) {
            return append + stem.substring(strip.length());
        }
        return stem.substring(0, stem.length() - strip.length()) + append;
    }

    /** Strip this rule's affix off a candidate form, returning the proposed root or {@code null}. */
    String stripFrom(String word) {
        if (prefix) {
            if (!word.startsWith(append)) {
                return null;
            }
            String tail = word.substring(append.length());
            String candidate = strip + tail;
            if (candidate.isEmpty()) {
                return null;
            }
            if (!condition.matcher(candidate).matches()) {
                return null;
            }
            return candidate;
        }
        if (!word.endsWith(append)) {
            return null;
        }
        String head = word.substring(0, word.length() - append.length());
        String candidate = head + strip;
        if (candidate.isEmpty()) {
            return null;
        }
        if (!condition.matcher(candidate).matches()) {
            return null;
        }
        return candidate;
    }

    private boolean matchesStem(String stem) {
        if (prefix) {
            if (!stem.startsWith(strip)) {
                return false;
            }
        } else {
            if (!stem.endsWith(strip)) {
                return false;
            }
        }
        return condition.matcher(stem).matches();
    }

    private static Pattern compileCondition(String condition, boolean prefix) {
        String normalized = ".".equals(condition) ? ".*" : condition;
        return Pattern.compile(prefix ? "^" + normalized + ".*" : ".*" + normalized + "$");
    }
}
