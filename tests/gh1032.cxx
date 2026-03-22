// Test for https://github.com/hunspell/hunspell/issues/1032
// HashMgr::remove double delete[] when add_with_affix shares astr pointer with
// aliased flags (AF directive present).
#include <cstdio>
#include <string>
#include "../src/hunspell/hunspell.hxx"

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s tests/gh1032\n", argv[0]);
    return 1;
  }
  std::string base(argv[1]);
  std::string aff = base + ".aff";
  std::string dic = base + ".dic";

  Hunspell h(aff.c_str(), dic.c_str());

  // "test" should be known and have suffix "s" via SFX A
  if (!h.spell(std::string("test"))) {
    fprintf(stderr, "FAIL: 'test' not in dictionary\n");
    return 1;
  }
  if (!h.spell(std::string("tests"))) {
    fprintf(stderr, "FAIL: 'tests' not in dictionary\n");
    return 1;
  }

  // add_with_affix: "x" gets same affix flags as "test"
  h.add_with_affix(std::string("x"), std::string("test"));

  if (!h.spell(std::string("x"))) {
    fprintf(stderr, "FAIL: 'x' not recognized after add_with_affix\n");
    return 1;
  }
  if (!h.spell(std::string("xs"))) {
    fprintf(stderr, "FAIL: 'xs' not recognized after add_with_affix\n");
    return 1;
  }

  // remove "test" - this used to delete[] the shared astr
  h.remove(std::string("test"));

  // "x" must still be spellable (would use-after-free without fix)
  if (!h.spell(std::string("x"))) {
    fprintf(stderr, "FAIL: 'x' not recognized after removing 'test'\n");
    return 1;
  }

  // remove "x" - this used to double-free without fix
  h.remove(std::string("x"));

  fprintf(stdout, "PASS: gh1032\n");
  return 0;
}
