package org.hunspell;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import org.junit.jupiter.api.Test;

class HunspellPortedCorpusTest {

    private static final Path CONDITION_AFF = Path.of("..", "tests", "condition.aff").normalize();
    private static final Path CONDITION_DIC = Path.of("..", "tests", "condition.dic").normalize();
    private static final Path CONDITION_GOOD = Path.of("..", "tests", "condition.good").normalize();
    private static final Path CONDITION_WRONG = Path.of("..", "tests", "condition.wrong").normalize();
    private static final Path CONDITION_UTF_AFF = Path.of("..", "tests", "condition_utf.aff").normalize();
    private static final Path CONDITION_UTF_DIC = Path.of("..", "tests", "condition_utf.dic").normalize();
    private static final Path CONDITION_UTF_GOOD = Path.of("..", "tests", "condition_utf.good").normalize();
    private static final Path CONDITION_UTF_WRONG = Path.of("..", "tests", "condition_utf.wrong").normalize();
    private static final Path SLASH_DIC = Path.of("..", "tests", "slash.dic").normalize();
    private static final Path SLASH_GOOD = Path.of("..", "tests", "slash.good").normalize();
    private static final Path BASE_AFF = Path.of("..", "tests", "base.aff").normalize();
    private static final Path BASE_DIC = Path.of("..", "tests", "base.dic").normalize();

    @Test
    void conditionGood_ofosuf1_isAccepted() {
        assertConditionAccepted("ofosuf1");
    }

    @Test
    void conditionGood_pre1ofo_isAccepted() {
        assertConditionAccepted("pre1ofo");
    }

    @Test
    void conditionGood_entertaining_isAccepted() {
        assertConditionAccepted("entertaining");
    }

    @Test
    void conditionGood_wries_isAccepted() {
        assertConditionAccepted("wries");
    }

    @Test
    void conditionGood_unwry_isAccepted() {
        assertConditionAccepted("unwry");
    }

    @Test
    void conditionGood_accentedWord_isAccepted() {
        assertConditionAccepted("érach");
    }

    @Test
    void conditionWrong_ofosuf4_isRejected() {
        assertConditionRejected("ofosuf4");
    }

    @Test
    void conditionWrong_pre4ofo_isRejected() {
        assertConditionRejected("pre4ofo");
    }

    @Test
    void conditionWrong_entertainning_isRejected() {
        assertConditionRejected("entertainning");
    }

    @Test
    void conditionWrong_gninnianretne_isRejected() {
        assertConditionRejected("gninnianretne");
    }

    @Test
    void slashGood_fraction_isAccepted() {
        assertSlashAccepted("1/2");
    }

    @Test
    void slashGood_httpPrefix_isAccepted() {
        assertSlashAccepted("http://");
    }

    @Test
    void conditionUtfGood_suf1_isAccepted() {
        assertConditionUtfAccepted("óőósuf1");
    }

    @Test
    void conditionUtfGood_pre1_isAccepted() {
        assertConditionUtfAccepted("pre1óőó");
    }

    @Test
    void conditionUtfGood_suf3_isAccepted() {
        assertConditionUtfAccepted("óőósuf3");
    }

    @Test
    void conditionUtfGood_pre7_isAccepted() {
        assertConditionUtfAccepted("pre7óőó");
    }

    @Test
    void conditionUtfGood_suf16_isAccepted() {
        assertConditionUtfAccepted("óőósuf16");
    }

    @Test
    void conditionUtfWrong_suf4_isRejected() {
        assertConditionUtfRejected("óőósuf4");
    }

    @Test
    void conditionUtfWrong_pre4_isRejected() {
        assertConditionUtfRejected("pre4óőó");
    }

    @Test
    void conditionUtfWrong_suf11_isRejected() {
        assertConditionUtfRejected("óőósuf11");
    }

    @Test
    void conditionUtfWrong_pre12_isRejected() {
        assertConditionUtfRejected("pre12óőó");
    }

    @Test
    void conditionUtfWrong_pre18_isRejected() {
        assertConditionUtfRejected("pre18óőó");
    }

    @Test
    void conditionCorpusGood_allWordsAccepted() {
        assertAllAccepted(CONDITION_AFF, CONDITION_DIC, CONDITION_GOOD, StandardCharsets.UTF_8);
    }

    @Test
    void conditionCorpusWrong_allWordsRejected() {
        assertAllRejected(CONDITION_AFF, CONDITION_DIC, CONDITION_WRONG, StandardCharsets.UTF_8);
    }

    @Test
    void conditionUtfCorpusGood_allWordsAccepted() {
        assertAllAccepted(CONDITION_UTF_AFF, CONDITION_UTF_DIC, CONDITION_UTF_GOOD, StandardCharsets.UTF_8);
    }

    @Test
    void conditionUtfCorpusWrong_allWordsRejected() {
        assertAllRejected(CONDITION_UTF_AFF, CONDITION_UTF_DIC, CONDITION_UTF_WRONG, StandardCharsets.UTF_8);
    }

    @Test
    void slashCorpusGood_allWordsAccepted() {
        assertAllAccepted(null, SLASH_DIC, SLASH_GOOD, StandardCharsets.ISO_8859_1);
    }

    @Test
    void baseGood_uncreated_isAccepted() {
        assertBaseAccepted("uncreated");
    }

    @Test
    void baseGood_implied_isAccepted() {
        assertBaseAccepted("implied");
    }

    @Test
    void baseGood_unnatural_isAccepted() {
        assertBaseAccepted("unnatural");
    }

    @Test
    void baseGood_conveyed_isAccepted() {
        assertBaseAccepted("conveyed");
    }

    @Test
    void baseGood_faqs_isAccepted() {
        assertBaseAccepted("FAQs");
    }

    @Test
    void baseGood_helloTitlecase_isAccepted() {
        assertBaseAccepted("Hello");
    }

    @Test
    void baseGood_helloUpper_isAccepted() {
        assertBaseAccepted("HELLO");
    }

    @Test
    void baseGood_hunspellUpperWithDots_isAccepted() {
        assertBaseAccepted("HUNSPELL...");
    }

    @Test
    void baseGood_textWithDot_isAccepted() {
        assertBaseAccepted("Text.");
    }

    @Test
    void baseWrong_nasaTitlecase_isRejected() {
        assertBaseRejected("Nasa");
    }

    @Test
    void baseWrong_tomorow_isRejected() {
        assertBaseRejected("tomorow");
    }

    @Test
    void baseWrong_sugesst_isRejected() {
        assertBaseRejected("sugesst");
    }

    private static void assertConditionAccepted(String word) {
        try (Hunspell hunspell = Hunspell.builder().affix(CONDITION_AFF).dictionary(CONDITION_DIC).build()) {
            assertTrue(hunspell.spell(word));
        }
    }

    private static void assertConditionRejected(String word) {
        try (Hunspell hunspell = Hunspell.builder().affix(CONDITION_AFF).dictionary(CONDITION_DIC).build()) {
            assertFalse(hunspell.spell(word));
        }
    }

    private static void assertSlashAccepted(String word) {
        try (Hunspell hunspell = Hunspell.builder().dictionary(SLASH_DIC).build()) {
            assertTrue(hunspell.spell(word));
        }
    }

    private static void assertConditionUtfAccepted(String word) {
        try (Hunspell hunspell = Hunspell.builder().affix(CONDITION_UTF_AFF).dictionary(CONDITION_UTF_DIC).build()) {
            assertTrue(hunspell.spell(word));
        }
    }

    private static void assertConditionUtfRejected(String word) {
        try (Hunspell hunspell = Hunspell.builder().affix(CONDITION_UTF_AFF).dictionary(CONDITION_UTF_DIC).build()) {
            assertFalse(hunspell.spell(word));
        }
    }

    private static void assertBaseAccepted(String word) {
        try (Hunspell hunspell = Hunspell.builder().affix(BASE_AFF).dictionary(BASE_DIC).build()) {
            assertTrue(hunspell.spell(word));
        }
    }

    private static void assertBaseRejected(String word) {
        try (Hunspell hunspell = Hunspell.builder().affix(BASE_AFF).dictionary(BASE_DIC).build()) {
            assertFalse(hunspell.spell(word));
        }
    }

    private static void assertAllAccepted(Path affix, Path dictionary, Path corpus, java.nio.charset.Charset charset) {
        List<String> words = loadCorpusWords(corpus, charset);
        try (Hunspell hunspell = build(affix, dictionary)) {
            for (String word : words) {
                assertTrue(hunspell.spell(word), () -> "Expected accepted: " + word);
            }
        }
    }

    private static void assertAllRejected(Path affix, Path dictionary, Path corpus, java.nio.charset.Charset charset) {
        List<String> words = loadCorpusWords(corpus, charset);
        try (Hunspell hunspell = build(affix, dictionary)) {
            for (String word : words) {
                assertFalse(hunspell.spell(word), () -> "Expected rejected: " + word);
            }
        }
    }

    private static Hunspell build(Path affix, Path dictionary) {
        Hunspell.Builder builder = Hunspell.builder().dictionary(dictionary);
        if (affix != null) {
            builder.affix(affix);
        }
        return builder.build();
    }

    private static List<String> loadCorpusWords(Path corpus, java.nio.charset.Charset charset) {
        try {
            return Files.readAllLines(corpus, charset).stream()
                .map(String::strip)
                .filter(line -> !line.isEmpty())
                .toList();
        } catch (IOException ex) {
            throw new AssertionError("Failed to read corpus file: " + corpus, ex);
        }
    }
}
