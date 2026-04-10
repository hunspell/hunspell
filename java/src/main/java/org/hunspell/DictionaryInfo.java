package org.hunspell;

public final class DictionaryInfo {
    private final String encoding;
    private final String version;
    private final int languageNumber;
    private final String wordCharacters;

    public DictionaryInfo(String encoding, String version, int languageNumber, String wordCharacters) {
        this.encoding = encoding;
        this.version = version;
        this.languageNumber = languageNumber;
        this.wordCharacters = wordCharacters;
    }

    public String encoding() {
        return encoding;
    }

    public String version() {
        return version;
    }

    public int languageNumber() {
        return languageNumber;
    }

    public String wordCharacters() {
        return wordCharacters;
    }
}
