// Regression for issue #616: runtime mutations use the same ICONV as spell().
#include <cstdio>
#include <string>
#include "../src/hunspell/hunspell.hxx"

int main(int argc, char** argv) {
  if (argc != 2)
    return 1;
  const std::string base(argv[1]);
  int failures = 0;
  auto expect = [&](bool ok, const std::string& label) {
    if (!ok) {
      fprintf(stderr, "FAIL: %s\n", label.c_str());
      ++failures;
    }
  };
  for (const auto& word : {"foo’bar", "foo'bar", "plain", "xfoo"}) {
    for (int method = 0; method < 3; ++method) {
      Hunspell h((base + ".aff").c_str(), (base + ".dic").c_str());
      const std::string raw(word);
      std::string converted;
      h.input_conv(raw, converted);
      expect(!h.spell(raw), "initially absent: " + raw);
      int ret = method == 0 ? h.add(raw) : method == 1 ? h.add_with_affix(raw, "seed") : h.add_with_flags(raw, "S");
      expect(ret == 0, "add returns success: " + raw);
      expect(h.spell(raw), "added raw: " + raw);
      // x -> y -> z must be applied only once, including by remove/re-add.
      if (raw != "xfoo")
        expect(h.spell(converted), "added normalized: " + raw);
      else
        expect(!h.spell(converted), "no double ICONV: " + raw);
      if (method != 0)
        expect(h.spell(raw + "s"), "inherited suffix: " + raw);
      expect(h.remove(raw) == 0, "remove returns success: " + raw);
      expect(!h.spell(raw), "removed: " + raw);
      expect(h.add(raw) == 0 && h.spell(raw), "re-add: " + raw);
    }
  }
  for (const auto& example : {"seed’root", "seed'root", "xseed"}) {
    Hunspell h((base + ".aff").c_str(), (base + ".dic").c_str());
    expect(h.add_with_affix("new’word", example) == 0, std::string("example: ") + example);
    expect(h.spell(std::string("new’word")) && h.spell(std::string("new'words")), std::string("example suffix: ") + example);
  }
  for (const auto& word : {"seed’root", "seed'root", "seed", "xseeds"}) {
    Hunspell h((base + ".aff").c_str(), (base + ".dic").c_str());
    expect(h.spell(std::string(word)), std::string("dictionary control: ") + word);
    expect(h.remove(word) == 0 && !h.spell(std::string(word)), std::string("dictionary removal: ") + word);
  }
  return failures ? 1 : 0;
}
