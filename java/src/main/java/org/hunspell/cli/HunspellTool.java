package org.hunspell.cli;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Path;
import org.hunspell.Hunspell;

public final class HunspellTool {
    private HunspellTool() {}

    public static void main(String[] args) throws IOException {
        String mode = "-a";
        Path dictionary = null;

        for (int i = 0; i < args.length; i++) {
            if ("-d".equals(args[i]) && i + 1 < args.length) {
                dictionary = resolveDictionary(args[++i]);
            } else if ("-l".equals(args[i]) || "-G".equals(args[i]) || "-a".equals(args[i])) {
                mode = args[i];
            }
        }

        if (dictionary == null) {
            throw new IllegalArgumentException("-d <dictionary.dic|base> is required");
        }

        try (Hunspell hunspell = Hunspell.builder().dictionary(dictionary).build();
             BufferedReader reader = new BufferedReader(new InputStreamReader(System.in))) {
            String line;
            while ((line = reader.readLine()) != null) {
                boolean correct = hunspell.spell(line);
                if ("-l".equals(mode) && !correct) {
                    System.out.println(line);
                } else if ("-G".equals(mode) && correct) {
                    System.out.println(line);
                } else if ("-a".equals(mode)) {
                    System.out.println(correct ? "*" : "& " + line + " 0 0: " + String.join(", ", hunspell.suggest(line)));
                }
            }
        }
    }

    private static Path resolveDictionary(String argument) {
        Path raw = Path.of(argument);
        if (raw.toString().endsWith(".dic")) {
            return raw;
        }
        return Path.of(argument + ".dic");
    }
}
