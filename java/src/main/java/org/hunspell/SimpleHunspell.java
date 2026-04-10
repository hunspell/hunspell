package org.hunspell;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

final class SimpleHunspell implements Hunspell {
    private final Set<String> words;
    private final int maxSuggestions;

    private SimpleHunspell(Set<String> words, int maxSuggestions) {
        this.words = words;
        this.maxSuggestions = maxSuggestions;
    }

    @Override
    public boolean spell(String word) {
        return words.contains(word);
    }

    @Override
    public SpellResult check(String word) {
        return new SpellResult(spell(word), false, false, null);
    }

    @Override
    public List<String> suggest(String word) {
        List<String> ranked = new ArrayList<>(words);
        ranked.sort(Comparator.comparingInt(candidate -> distance(word, candidate)));
        if (ranked.size() > maxSuggestions) {
            ranked = ranked.subList(0, maxSuggestions);
        }
        return Collections.unmodifiableList(ranked);
    }

    @Override
    public List<String> suffixSuggest(String rootWord) {
        List<String> matches = words.stream().filter(w -> w.endsWith(rootWord)).sorted().toList();
        return Collections.unmodifiableList(matches);
    }

    @Override
    public int addDictionary(Path dicPath) {
        int before = words.size();
        loadWords(dicPath, words);
        return words.size() - before;
    }

    @Override
    public DictionaryInfo info() {
        return new DictionaryInfo("UTF-8", "java-port-dev", 0, "");
    }

    @Override
    public void close() {
        words.clear();
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
            if (affPath != null && !Files.exists(affPath)) {
                throw new HunspellParseException("Affix file does not exist: " + affPath);
            }

            Set<String> words = new HashSet<>();
            loadWords(primaryDictionary, words);
            for (Path extraDictionary : extraDictionaries) {
                loadWords(extraDictionary, words);
            }
            return new SimpleHunspell(words, maxSuggestions);
        }
    }

    private static void loadWords(Path dicPath, Set<String> words) {
        try {
            List<String> lines = Files.readAllLines(dicPath);
            int start = 0;
            if (!lines.isEmpty() && lines.get(0).strip().matches("\\d+")) {
                start = 1;
            }
            for (int i = start; i < lines.size(); i++) {
                String line = lines.get(i).strip();
                if (!line.isEmpty()) {
                    words.add(line.split("/", 2)[0]);
                }
            }
        } catch (IOException ex) {
            throw new HunspellIoException("Unable to read dictionary: " + dicPath, ex);
        }
    }
}
