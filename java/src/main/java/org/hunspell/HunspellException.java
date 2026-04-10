package org.hunspell;

public class HunspellException extends RuntimeException {
    public HunspellException(String message) {
        super(message);
    }

    public HunspellException(String message, Throwable cause) {
        super(message, cause);
    }
}
