package org.hunspell;

import java.nio.file.Path;
import java.util.List;

public interface Hunspell extends AutoCloseable {
    static Builder builder() {
        return new SimpleHunspell.BuilderImpl();
    }

    boolean spell(String word);

    SpellResult check(String word);

    List<String> suggest(String word);

    List<String> suffixSuggest(String rootWord);

    int addDictionary(Path dicPath);

    DictionaryInfo info();

    @Override
    void close();

    interface Builder {
        Builder affix(Path affPath);

        Builder dictionary(Path dicPath);

        Builder addDictionary(Path dicPath);

        Builder key(String key);

        Builder strictAffixParsing(boolean strict);

        Builder maxSuggestions(int max);

        Hunspell build();
    }
}
