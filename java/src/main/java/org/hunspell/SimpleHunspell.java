package org.hunspell;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.Set;

/**
 * Façade implementation of {@link Hunspell} that wires together the manager
 * classes mirroring the C++ Hunspell architecture:
 * <ul>
 *   <li>{@link AffixManager} ↔ {@code AffixMgr}/{@code affentry.cxx}</li>
 *   <li>{@link HashManager} ↔ {@code HashMgr}</li>
 *   <li>This class plays the role of {@code Hunspell}/{@code HunspellImpl} in
 *       {@code hunspell.cxx}, exposing the user-facing spell/check/suggest
 *       surface required by the v1 Java API.</li>
 * </ul>
 */
final class SimpleHunspell implements Hunspell {

    private final AffixManager affixManager;
    private final HashManager hashManager;
    private final int maxSuggestions;

    private SimpleHunspell(AffixManager affixManager, HashManager hashManager, int maxSuggestions) {
        this.affixManager = affixManager;
        this.hashManager = hashManager;
        this.maxSuggestions = maxSuggestions;
    }

    @Override
    public boolean spell(String word) {
        return resolveStem(word) != null;
    }

    @Override
    public SpellResult check(String word) {
        String stem = resolveStem(word);
        boolean correct = stem != null;
        return new SpellResult(correct, false, false, correct ? stem : null);
    }

    @Override
    public List<String> suggest(String word) {
        // The current suggestion ranking is intentionally simple (Levenshtein over
        // the loaded stems plus their visible derived forms) until the parity-grade
        // SuggestManager port lands. This still preserves API contract.
        Set<String> candidates = new LinkedHashSet<>();
        for (var bucket : hashManager.all()) {
            candidates.add(bucket.getKey());
        }
        List<String> ranked = new ArrayList<>(candidates);
        ranked.sort(Comparator
            .comparingInt((String candidate) -> distance(word, candidate))
            .thenComparing(Comparator.naturalOrder()));
        if (ranked.size() > maxSuggestions) {
            ranked = ranked.subList(0, maxSuggestions);
        }
        return Collections.unmodifiableList(ranked);
    }

    @Override
    public List<String> suffixSuggest(String rootWord) {
        List<String> matches = new ArrayList<>();
        for (var bucket : hashManager.all()) {
            String stem = bucket.getKey();
            if (stem.startsWith(rootWord)) {
                matches.add(stem);
            }
        }
        Collections.sort(matches);
        return Collections.unmodifiableList(matches);
    }

    @Override
    public int addDictionary(Path dicPath) {
        int before = hashManager.size();
        hashManager.load(dicPath, affixManager.charset());
        return hashManager.size() - before;
    }

    @Override
    public DictionaryInfo info() {
        return new DictionaryInfo(affixManager.encoding(), "java-port-dev", 0, affixManager.wordChars());
    }

    @Override
    public void close() {
        hashManager.clear();
    }

    /**
     * Walks the same lookup ladder as {@code Hunspell::spell}: exact match first,
     * then case/punctuation normalisation variants, then affix recursion. The
     * returned stem is the resolved root (or {@code null} if rejected).
     */
    private String resolveStem(String word) {
        if (word == null || word.isEmpty()) {
            return null;
        }
        String stem = lookupVariant(word);
        if (stem != null) {
            return stem;
        }
        String withoutTrailingDots = stripTrailingDots(word);
        if (!withoutTrailingDots.equals(word) && !withoutTrailingDots.isEmpty()) {
            stem = resolveStem(withoutTrailingDots);
            if (stem != null) {
                return stem;
            }
        }
        if (isTitleCase(word)) {
            String lower = word.toLowerCase(Locale.ROOT);
            stem = lookupVariant(lower);
            if (stem != null) {
                return stem;
            }
        } else if (isAllUpper(word)) {
            String lower = word.toLowerCase(Locale.ROOT);
            stem = lookupVariant(lower);
            if (stem != null) {
                return stem;
            }
            String capitalized = capitalize(lower);
            stem = lookupVariant(capitalized);
            if (stem != null) {
                return stem;
            }
        }
        return null;
    }

    private String lookupVariant(String word) {
        if (hashManager.contains(word)) {
            return word;
        }
        HashManager.Entry hit = affixManager.affixCheck(word, hashManager);
        return hit == null ? null : hit.stem();
    }

    private static int distance(String left, String right) {
        int[][] dp = new int[left.length() + 1][right.length() + 1];
        for (int i = 0; i <= left.length(); i++) {
            dp[i][0] = i;
        }
        for (int j = 0; j <= right.length(); j++) {
            dp[0][j] = j;
        }
        for (int i = 1; i <= left.length(); i++) {
            for (int j = 1; j <= right.length(); j++) {
                int substitutionCost = left.charAt(i - 1) == right.charAt(j - 1) ? 0 : 1;
                dp[i][j] = Math.min(
                    Math.min(dp[i - 1][j] + 1, dp[i][j - 1] + 1),
                    dp[i - 1][j - 1] + substitutionCost);
            }
        }
        return dp[left.length()][right.length()];
    }

    private static String stripTrailingDots(String word) {
        int end = word.length();
        while (end > 0 && word.charAt(end - 1) == '.') {
            end--;
        }
        return end == word.length() ? word : word.substring(0, end);
    }

    private static boolean isTitleCase(String word) {
        if (word.length() < 2) {
            return false;
        }
        if (!Character.isUpperCase(word.charAt(0))) {
            return false;
        }
        for (int i = 1; i < word.length(); i++) {
            if (!Character.isLowerCase(word.charAt(i))) {
                return false;
            }
        }
        return true;
    }

    private static boolean isAllUpper(String word) {
        boolean hasLetter = false;
        for (int i = 0; i < word.length(); i++) {
            char ch = word.charAt(i);
            if (Character.isLetter(ch)) {
                hasLetter = true;
                if (!Character.isUpperCase(ch)) {
                    return false;
                }
            }
        }
        return hasLetter;
    }

    private static String capitalize(String word) {
        if (word.isEmpty()) {
            return word;
        }
        return Character.toUpperCase(word.charAt(0)) + word.substring(1);
    }

    static final class BuilderImpl implements Hunspell.Builder {
        private Path affPath;
        private Path primaryDictionary;
        private final List<Path> extraDictionaries = new ArrayList<>();
        private int maxSuggestions = 10;

        @Override
        public Hunspell.Builder affix(Path affPath) {
            this.affPath = Objects.requireNonNull(affPath, "affPath");
            return this;
        }

        @Override
        public Hunspell.Builder dictionary(Path dicPath) {
            this.primaryDictionary = Objects.requireNonNull(dicPath, "dicPath");
            return this;
        }

        @Override
        public Hunspell.Builder addDictionary(Path dicPath) {
            this.extraDictionaries.add(Objects.requireNonNull(dicPath, "dicPath"));
            return this;
        }

        @Override
        public Hunspell.Builder key(String key) {
            return this;
        }

        @Override
        public Hunspell.Builder strictAffixParsing(boolean strict) {
            return this;
        }

        @Override
        public Hunspell.Builder maxSuggestions(int max) {
            this.maxSuggestions = Math.max(1, max);
            return this;
        }

        @Override
        public Hunspell build() {
            if (primaryDictionary == null) {
                throw new HunspellStateException("Primary dictionary is required");
            }
            AffixManager affixManager = new AffixManager();
            if (affPath != null) {
                if (!Files.exists(affPath)) {
                    throw new HunspellParseException("Affix file does not exist: " + affPath);
                }
                affixManager.load(affPath);
            }
            HashManager hashManager = new HashManager(affixManager.flagMode());
            hashManager.load(primaryDictionary, affixManager.charset());
            for (Path extraDictionary : extraDictionaries) {
                hashManager.load(extraDictionary, affixManager.charset());
            }
            return new SimpleHunspell(affixManager, hashManager, maxSuggestions);
        }
    }
}
