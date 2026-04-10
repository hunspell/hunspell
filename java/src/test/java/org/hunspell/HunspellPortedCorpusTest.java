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
    private static final Path BASE_UTF_AFF = Path.of("..", "tests", "base_utf.aff").normalize();
    private static final Path BASE_UTF_DIC = Path.of("..", "tests", "base_utf.dic").normalize();
    private static final Path AFFIXES_AFF = Path.of("..", "tests", "affixes.aff").normalize();
    private static final Path AFFIXES_DIC = Path.of("..", "tests", "affixes.dic").normalize();
    private static final Path AFFIXES_GOOD = Path.of("..", "tests", "affixes.good").normalize();
    private static final Path FLAG_AFF = Path.of("..", "tests", "flag.aff").normalize();
    private static final Path FLAG_DIC = Path.of("..", "tests", "flag.dic").normalize();
    private static final Path FLAG_GOOD = Path.of("..", "tests", "flag.good").normalize();
    private static final Path FLAGLONG_AFF = Path.of("..", "tests", "flaglong.aff").normalize();
    private static final Path FLAGLONG_DIC = Path.of("..", "tests", "flaglong.dic").normalize();
    private static final Path FLAGLONG_GOOD = Path.of("..", "tests", "flaglong.good").normalize();
    private static final Path FLAGNUM_AFF = Path.of("..", "tests", "flagnum.aff").normalize();
    private static final Path FLAGNUM_DIC = Path.of("..", "tests", "flagnum.dic").normalize();
    private static final Path FLAGNUM_GOOD = Path.of("..", "tests", "flagnum.good").normalize();
    private static final Path FLAGUTF8_AFF = Path.of("..", "tests", "flagutf8.aff").normalize();
    private static final Path FLAGUTF8_DIC = Path.of("..", "tests", "flagutf8.dic").normalize();
    private static final Path FLAGUTF8_GOOD = Path.of("..", "tests", "flagutf8.good").normalize();

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

    @Test
    void affixesCorpusGood_allWordsAccepted() {
        assertAllAccepted(AFFIXES_AFF, AFFIXES_DIC, AFFIXES_GOOD, StandardCharsets.ISO_8859_1);
    }

    @Test
    void affixesGood_reworkedCrossProductAccepted() {
        try (Hunspell hunspell = Hunspell.builder().affix(AFFIXES_AFF).dictionary(AFFIXES_DIC).build()) {
            assertTrue(hunspell.spell("rework"));
            assertTrue(hunspell.spell("worked"));
            assertTrue(hunspell.spell("reworked"));
            assertTrue(hunspell.spell("tried"));
        }
    }

    @Test
    void flagCorpusGood_singleCharFlagsAccepted() {
        assertAllAccepted(FLAG_AFF, FLAG_DIC, FLAG_GOOD, StandardCharsets.ISO_8859_1);
    }

    @Test
    void flagGood_continuationClassChainProducesNestedSuffix() {
        try (Hunspell hunspell = Hunspell.builder().affix(FLAG_AFF).dictionary(FLAG_DIC).build()) {
            assertTrue(hunspell.spell("foosbar"));
            assertTrue(hunspell.spell("foosbaz"));
            assertTrue(hunspell.spell("unfoosbar"));
        }
    }

    @Test
    void flagLongCorpusGood_twoCharFlagsAccepted() {
        assertAllAccepted(FLAGLONG_AFF, FLAGLONG_DIC, FLAGLONG_GOOD, StandardCharsets.ISO_8859_1);
    }

    @Test
    void flagNumCorpusGood_decimalFlagsAccepted() {
        assertAllAccepted(FLAGNUM_AFF, FLAGNUM_DIC, FLAGNUM_GOOD, StandardCharsets.ISO_8859_1);
    }

    @Test
    void flagUtf8CorpusGood_unicodeFlagsAccepted() {
        assertAllAccepted(FLAGUTF8_AFF, FLAGUTF8_DIC, FLAGUTF8_GOOD, StandardCharsets.UTF_8);
    }

    @Test
    void baseUtfGood_unicodeStemAccepted() {
        try (Hunspell hunspell = Hunspell.builder().affix(BASE_UTF_AFF).dictionary(BASE_UTF_DIC).build()) {
            assertTrue(hunspell.spell("created"));
            assertTrue(hunspell.spell("uncreated"));
            assertTrue(hunspell.spell("conveyed"));
            assertTrue(hunspell.spell("FAQs"));
            assertTrue(hunspell.spell("Hello"));
            assertTrue(hunspell.spell("HELLO"));
            assertTrue(hunspell.spell("NASA"));
        }
    }

    @Test
    void baseUtfWrong_misspelledFormsRejected() {
        try (Hunspell hunspell = Hunspell.builder().affix(BASE_UTF_AFF).dictionary(BASE_UTF_DIC).build()) {
            assertFalse(hunspell.spell("loooked"));
            assertFalse(hunspell.spell("hlelo"));
            assertFalse(hunspell.spell("tomorow"));
            assertFalse(hunspell.spell("Nasa"));
        }
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
