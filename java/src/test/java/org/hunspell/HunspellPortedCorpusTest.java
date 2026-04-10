package org.hunspell;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.nio.file.Path;
import org.junit.jupiter.api.Test;

class HunspellPortedCorpusTest {

    private static final Path CONDITION_AFF = Path.of("..", "tests", "condition.aff").normalize();
    private static final Path CONDITION_DIC = Path.of("..", "tests", "condition.dic").normalize();
    private static final Path SLASH_DIC = Path.of("..", "tests", "slash.dic").normalize();

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
}
