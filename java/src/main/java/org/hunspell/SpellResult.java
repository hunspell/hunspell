package org.hunspell;

import java.util.Optional;

public final class SpellResult {
    private final boolean correct;
    private final boolean compound;
    private final boolean forbidden;
    private final String root;

    public SpellResult(boolean correct, boolean compound, boolean forbidden, String root) {
        this.correct = correct;
        this.compound = compound;
        this.forbidden = forbidden;
        this.root = root;
    }

    public boolean correct() {
        return correct;
    }

    public boolean compound() {
        return compound;
    }

    public boolean forbidden() {
        return forbidden;
    }

    public Optional<String> root() {
        return Optional.ofNullable(root);
    }
}
