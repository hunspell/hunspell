package org.hunspell.cli;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.nio.file.Files;
import java.nio.file.Path;
import org.junit.jupiter.api.Test;

class HunspellToolTest {

    @Test
    void listModePrintsOnlyMisspelledWords() throws IOException {
        Path dictionary = writeDictionary("2", "hello", "world");

        String output = runTool(new String[] {"-d", dictionary.toString(), "-l"}, "hello\nwrong\nworld\n");

        assertEquals("wrong\n", output);
    }

    @Test
    void goodModePrintsOnlyCorrectWords() throws IOException {
        Path dictionary = writeDictionary("2", "hello", "world");

        String output = runTool(new String[] {"-d", dictionary.toString(), "-G"}, "hello\nwrong\nworld\n");

        assertEquals("hello\nworld\n", output);
    }

    @Test
    void goodModeResolvesDictionaryWithoutDicExtension() throws IOException {
        Path tempDir = Files.createTempDirectory("hunspell-cli");
        Path dictionaryBase = tempDir.resolve("slash");
        Files.writeString(dictionaryBase.resolveSibling("slash.dic"), "2\n1\\/2\nhttp:\\/\\/\n");

        String output = runTool(new String[] {"-d", dictionaryBase.toString(), "-G"}, "1/2\nhttp://\nhttps://\n");

        assertEquals("1/2\nhttp://\n", output);
    }

    @Test
    void goodModeLoadsSiblingAffixFileWhenUsingBasePath() throws IOException {
        Path tempDir = Files.createTempDirectory("hunspell-cli");
        Path dictionaryBase = tempDir.resolve("mini");
        Files.writeString(dictionaryBase.resolveSibling("mini.aff"), "SET UTF-8\nSFX S N 1\nSFX S 0 s .\n");
        Files.writeString(dictionaryBase.resolveSibling("mini.dic"), "1\ncat/S\n");

        String output = runTool(new String[] {"-d", dictionaryBase.toString(), "-G"}, "cat\ncats\ndog\n");

        assertEquals("cat\ncats\n", output);
    }

    private static String runTool(String[] args, String input) throws IOException {
        ByteArrayInputStream in = new ByteArrayInputStream(input.getBytes());
        ByteArrayOutputStream out = new ByteArrayOutputStream();

        java.io.InputStream originalIn = System.in;
        PrintStream originalOut = System.out;
        try {
            System.setIn(in);
            System.setOut(new PrintStream(out));
            HunspellTool.main(args);
        } finally {
            System.setIn(originalIn);
            System.setOut(originalOut);
        }

        return out.toString();
    }

    private static Path writeDictionary(String... lines) throws IOException {
        Path file = Files.createTempFile("hunspell-cli", ".dic");
        Files.write(file, String.join(System.lineSeparator(), lines).concat(System.lineSeparator()).getBytes());
        file.toFile().deleteOnExit();
        return file;
    }
}
