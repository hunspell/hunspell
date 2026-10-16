// Regression for issue #1155: personal exclusions must also cover forms
// produced by affix rules, without excluding the unaffixed dictionary word.
#include <cstdio>
#include <string>
#include "../src/hunspell/hunspell.hxx"

int main(int argc, char** argv) {
  if (argc < 2)
    return 1;
  std::string base(argv[1]);
  Hunspell h((base + ".aff").c_str(), (base + ".dic").c_str());
  int failures = 0;
  auto check = [&](const char* word, bool expected) {
    if (h.spell(word) != expected) {
      fprintf(stderr, "FAIL: spell(%s) should be %s\n", word, expected ? "true" : "false");
      ++failures;
    }
  };

  check("afterward", true);
  check("afterwards", true);
  h.remove("afterwards");
  check("afterwards", false);
  check("afterward", true);
  h.remove("afterwards");
  check("afterwards", false);
  h.add("afterwards");
  check("afterwards", true);
  check("afterward", true);

  h.remove("facade");
  check("facade", false);
  h.add("facade");
  check("facade", true);

  check("cafés", true);
  h.remove("cafés");
  check("cafés", false);
  check("café", true);
  h.add("cafés");
  check("cafés", true);

  return failures ? 1 : 0;
}
