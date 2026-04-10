package org.hunspell;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Pattern;

final class SimpleHunspell implements Hunspell {
    private final Set<String> words;
    private final Map<String, String> roots;
    private final int maxSuggestions;
    private final String encoding;
    private final String wordChars;

    private SimpleHunspell(Set<String> words, Map<String, String> roots, int maxSuggestions, String encoding, String wordChars) {
        this.words = words;
        this.roots = roots;
        this.maxSuggestions = maxSuggestions;
        this.encoding = encoding;
        this.wordChars = wordChars;
    }

    @Override
    public boolean spell(String word) {
        return spellNormalized(word);
    }

    @Override
    public SpellResult check(String word) {
        boolean correct = spell(word);
        String root = correct ? resolveRoot(word) : null;
        return new SpellResult(correct, false, false, root);
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
        loadWords(dicPath, words, roots, null);
        return words.size() - before;
    }

    @Override
    public DictionaryInfo info() {
        return new DictionaryInfo(encoding, "java-port-dev", 0, wordChars);
    }

    @Override
    public void close() {
        words.clear();
        roots.clear();
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

    private String resolveRoot(String word) {
        String normalized = normalizeForLookup(word);
        return roots.get(normalized);
    }

    private boolean spellNormalized(String word) {
        if (word == null || word.isEmpty()) {
            return false;
        }
        return normalizeForLookup(word) != null;
    }

    private String normalizeForLookup(String word) {
        if (words.contains(word)) {
            return word;
        }
        String withoutTrailingDots = stripTrailingDots(word);
        if (!withoutTrailingDots.equals(word)) {
            String normalized = normalizeForLookup(withoutTrailingDots);
            if (normalized != null) {
                return normalized;
            }
        }
        if (isTitleCase(word)) {
            String lower = word.toLowerCase(Locale.ROOT);
            return words.contains(lower) ? lower : null;
        }
        if (isAllUpper(word)) {
            String lower = word.toLowerCase(Locale.ROOT);
            if (words.contains(lower)) {
                return lower;
            }
            String capitalized = capitalize(lower);
            return words.contains(capitalized) ? capitalized : null;
        }
        return null;
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

            AffixData affixData = null;
            if (affPath != null) {
                if (!Files.exists(affPath)) {
                    throw new HunspellParseException("Affix file does not exist: " + affPath);
                }
                affixData = loadAffixes(affPath);
            }

            Set<String> words = new HashSet<>();
            Map<String, String> roots = new HashMap<>();
            loadWords(primaryDictionary, words, roots, affixData);
            for (Path extraDictionary : extraDictionaries) {
                loadWords(extraDictionary, words, roots, affixData);
            }
            String encoding = affixData != null ? affixData.encoding : "UTF-8";
            String wordChars = affixData != null ? affixData.wordChars : "";
            return new SimpleHunspell(words, roots, maxSuggestions, encoding, wordChars);
        }
    }

    private static void loadWords(Path dicPath, Set<String> words, Map<String, String> roots, AffixData affixData) {
        try {
            Charset dictionaryCharset = affixData != null ? affixData.charset : StandardCharsets.ISO_8859_1;
            List<String> lines = Files.readAllLines(dicPath, dictionaryCharset);
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
                    roots.putIfAbsent(entry.stem(), entry.stem());
                    if (affixData != null) {
                        Map<String, String> generated = affixData.generate(entry);
                        words.addAll(generated.keySet());
                        generated.forEach(roots::putIfAbsent);
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
        if ("/".equals(line)) {
            return "/";
        }
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
            byte[] bytes = Files.readAllBytes(affPath);
            String initial = new String(bytes, StandardCharsets.ISO_8859_1);
            Charset affixCharset = detectDeclaredCharset(initial);
            String decoded = new String(bytes, affixCharset);
            List<String> lines = decoded.lines().toList();
            AffixData data = new AffixData(affixCharset);
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

    private static Charset detectDeclaredCharset(String affContent) {
        for (String rawLine : affContent.lines().toList()) {
            String line = rawLine.strip();
            if (line.isEmpty() || line.startsWith("#")) {
                continue;
            }
            String[] parts = line.split("\\s+");
            if (parts.length >= 2 && "SET".equals(parts[0])) {
                try {
                    return Charset.forName(parts[1]);
                } catch (IllegalArgumentException ignored) {
                    return StandardCharsets.ISO_8859_1;
                }
            }
        }
        return StandardCharsets.ISO_8859_1;
    }

    private record DictionaryEntry(String stem, String flags) {
    }

    private static final class AffixData {
        private String encoding = "UTF-8";
        private String wordChars = "";
        private final Charset charset;
        private final Map<Character, List<AffixRule>> prefixes = new HashMap<>();
        private final Map<Character, List<AffixRule>> suffixes = new HashMap<>();

        private AffixData(Charset charset) {
            this.charset = charset;
        }

        private void addRule(char flag, boolean prefix, AffixRule rule) {
            Map<Character, List<AffixRule>> target = prefix ? prefixes : suffixes;
            target.computeIfAbsent(flag, ignored -> new ArrayList<>()).add(rule);
        }

        private Map<String, String> generate(DictionaryEntry entry) {
            Map<String, String> generated = new HashMap<>();
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
                    generated.putIfAbsent(form, entry.stem());
                    prefixed.add(form);
                }
            }

            for (AffixRule rule : suffixRules) {
                String form = rule.apply(entry.stem());
                if (form != null) {
                    generated.putIfAbsent(form, entry.stem());
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
                        generated.putIfAbsent(combined, entry.stem());
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
