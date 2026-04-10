package org.hunspell;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Pattern;

final class SimpleHunspell implements Hunspell {
    private final Set<String> words;
    private final int maxSuggestions;
    private final String encoding;
    private final String wordChars;

    private SimpleHunspell(Set<String> words, int maxSuggestions, String encoding, String wordChars) {
        this.words = words;
        this.maxSuggestions = maxSuggestions;
        this.encoding = encoding;
        this.wordChars = wordChars;
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
        List<String> matches = words.stream().filter(w -> w.startsWith(rootWord)).sorted().toList();
        return Collections.unmodifiableList(matches);
    }

    @Override
    public int addDictionary(Path dicPath) {
        int before = words.size();
        loadWords(dicPath, words, null);
        return words.size() - before;
    }

    @Override
    public DictionaryInfo info() {
        return new DictionaryInfo(encoding, "java-port-dev", 0, wordChars);
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

            AffixData affixData = null;
            if (affPath != null) {
                if (!Files.exists(affPath)) {
                    throw new HunspellParseException("Affix file does not exist: " + affPath);
                }
                affixData = loadAffixes(affPath);
            }

            Set<String> words = new HashSet<>();
            loadWords(primaryDictionary, words, affixData);
            for (Path extraDictionary : extraDictionaries) {
                loadWords(extraDictionary, words, affixData);
            }
            String encoding = affixData != null ? affixData.encoding : "UTF-8";
            String wordChars = affixData != null ? affixData.wordChars : "";
            return new SimpleHunspell(words, maxSuggestions, encoding, wordChars);
        }
    }

    private static void loadWords(Path dicPath, Set<String> words, AffixData affixData) {
        try {
            List<String> lines = Files.readAllLines(dicPath, StandardCharsets.ISO_8859_1);
            int start = 0;
            if (!lines.isEmpty() && lines.get(0).strip().matches("\\d+")) {
                start = 1;
            }
            for (int i = start; i < lines.size(); i++) {
                String line = lines.get(i).strip();
                if (!line.isEmpty() && !line.startsWith("#")) {
                    DictionaryEntry entry = parseEntry(line);
                    if (entry.stem().isEmpty()) {
                        continue;
                    }
                    words.add(entry.stem());
                    if (affixData != null) {
                        words.addAll(affixData.generate(entry));
                    }
                }
            }
        } catch (IOException ex) {
            throw new HunspellIoException("Unable to read dictionary: " + dicPath, ex);
        }
    }

    private static DictionaryEntry parseEntry(String line) {
        String stem = parseStem(line);
        String flags = parseFlags(line);
        return new DictionaryEntry(stem, flags);
    }

    private static String parseStem(String line) {
        StringBuilder stem = new StringBuilder();
        boolean escaped = false;
        for (int i = 0; i < line.length(); i++) {
            char current = line.charAt(i);
            if (escaped) {
                stem.append(current);
                escaped = false;
                continue;
            }
            if (current == '\\') {
                escaped = true;
                continue;
            }
            if (current == '/' || Character.isWhitespace(current)) {
                break;
            }
            stem.append(current);
        }
        return stem.toString();
    }

    private static String parseFlags(String line) {
        int slashIndex = findFirstUnescapedSlash(line);
        if (slashIndex < 0 || slashIndex + 1 >= line.length()) {
            return "";
        }
        int end = slashIndex + 1;
        while (end < line.length() && !Character.isWhitespace(line.charAt(end))) {
            end++;
        }
        return line.substring(slashIndex + 1, end);
    }

    private static int findFirstUnescapedSlash(String text) {
        boolean escaped = false;
        for (int i = 0; i < text.length(); i++) {
            char ch = text.charAt(i);
            if (escaped) {
                escaped = false;
                continue;
            }
            if (ch == '\\') {
                escaped = true;
                continue;
            }
            if (ch == '/') {
                return i;
            }
        }
        return -1;
    }

    private static AffixData loadAffixes(Path affPath) {
        try {
            List<String> lines = Files.readAllLines(affPath, StandardCharsets.ISO_8859_1);
            AffixData data = new AffixData();
            for (int i = 0; i < lines.size(); i++) {
                String line = lines.get(i).strip();
                if (line.isEmpty() || line.startsWith("#")) {
                    continue;
                }

                String[] parts = line.split("\\s+");
                if (parts.length == 0) {
                    continue;
                }

                if ("SET".equals(parts[0]) && parts.length >= 2) {
                    data.encoding = parts[1];
                    continue;
                }
                if ("WORDCHARS".equals(parts[0]) && parts.length >= 2) {
                    data.wordChars = line.substring("WORDCHARS".length()).strip();
                    continue;
                }
                if (("PFX".equals(parts[0]) || "SFX".equals(parts[0])) && parts.length >= 4
                    && parts[3].matches("\\d+")) {
                    boolean isPrefix = "PFX".equals(parts[0]);
                    char flag = parts[1].charAt(0);
                    boolean cross = "Y".equals(parts[2]);
                    int count = Integer.parseInt(parts[3]);
                    for (int j = 0; j < count && i + 1 < lines.size(); j++) {
                        i++;
                        String ruleLine = lines.get(i).strip();
                        if (ruleLine.isEmpty() || ruleLine.startsWith("#")) {
                            j--;
                            continue;
                        }
                        String[] rule = ruleLine.split("\\s+");
                        if (rule.length < 5) {
                            continue;
                        }
                        String strip = "0".equals(rule[2]) ? "" : rule[2];
                        String append = "0".equals(rule[3]) ? "" : rule[3];
                        String condition = rule[4];
                        data.addRule(flag, isPrefix, new AffixRule(isPrefix, cross, strip, append, condition));
                    }
                }
            }
            return data;
        } catch (IOException ex) {
            throw new HunspellIoException("Unable to read affix file: " + affPath, ex);
        }
    }

    private record DictionaryEntry(String stem, String flags) {
    }

    private static final class AffixData {
        private String encoding = "UTF-8";
        private String wordChars = "";
        private final Map<Character, List<AffixRule>> prefixes = new HashMap<>();
        private final Map<Character, List<AffixRule>> suffixes = new HashMap<>();

        private void addRule(char flag, boolean prefix, AffixRule rule) {
            Map<Character, List<AffixRule>> target = prefix ? prefixes : suffixes;
            target.computeIfAbsent(flag, ignored -> new ArrayList<>()).add(rule);
        }

        private Set<String> generate(DictionaryEntry entry) {
            Set<String> generated = new HashSet<>();
            List<AffixRule> prefixRules = new ArrayList<>();
            List<AffixRule> suffixRules = new ArrayList<>();
            for (int i = 0; i < entry.flags().length(); i++) {
                char flag = entry.flags().charAt(i);
                prefixRules.addAll(prefixes.getOrDefault(flag, List.of()));
                suffixRules.addAll(suffixes.getOrDefault(flag, List.of()));
            }

            List<String> prefixed = new ArrayList<>();
            prefixed.add(entry.stem());
            for (AffixRule rule : prefixRules) {
                String form = rule.apply(entry.stem());
                if (form != null) {
                    generated.add(form);
                    prefixed.add(form);
                }
            }

            for (AffixRule rule : suffixRules) {
                String form = rule.apply(entry.stem());
                if (form != null) {
                    generated.add(form);
                }
            }

            for (AffixRule prefix : prefixRules) {
                if (!prefix.crossProduct()) {
                    continue;
                }
                String prefixedStem = prefix.apply(entry.stem());
                if (prefixedStem == null) {
                    continue;
                }
                for (AffixRule suffix : suffixRules) {
                    if (!suffix.crossProduct()) {
                        continue;
                    }
                    String combined = suffix.apply(prefixedStem);
                    if (combined != null) {
                        generated.add(combined);
                    }
                }
            }
            return generated;
        }
    }

    private static final class AffixRule {
        private final boolean prefix;
        private final boolean crossProduct;
        private final String strip;
        private final String append;
        private final Pattern conditionPattern;

        private AffixRule(boolean prefix, boolean crossProduct, String strip, String append, String condition) {
            this.prefix = prefix;
            this.crossProduct = crossProduct;
            this.strip = strip;
            this.append = append;
            String normalizedCondition = ".".equals(condition) ? ".*" : condition;
            this.conditionPattern = Pattern.compile(prefix ? "^" + normalizedCondition + ".*" : ".*" + normalizedCondition + "$");
        }

        private boolean crossProduct() {
            return crossProduct;
        }

        private String apply(String stem) {
            if (!conditionPattern.matcher(stem).matches()) {
                return null;
            }
            if (prefix) {
                if (!stem.startsWith(strip)) {
                    return null;
                }
                return append + stem.substring(strip.length());
            }
            if (!stem.endsWith(strip)) {
                return null;
            }
            return stem.substring(0, stem.length() - strip.length()) + append;
        }
    }
}
