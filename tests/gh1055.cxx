// COMPLEXPREFIXES must produce usable stem fields without reordering fields.
#include <cstdio>
#include <string>
#include <vector>
#include "../src/hunspell/hunspell.hxx"

int main(int argc, char** argv) {
  if (argc != 2)
    return 1;
  const std::string dir(argv[1]);
  int failures = 0;
  auto run = [&](const std::string& fixture, const std::string& root, const std::string& stem, const std::string& data) {
    const std::string base = dir + "/" + fixture;
    Hunspell h((base + ".aff").c_str(), (base + ".dic").c_str());
    const std::vector<std::string> words = {root, "met" + root, "tekmet" + root, root + "_test_"};
    const std::string field = "st:" + stem;
    const bool alias = fixture == "alias3";
    const std::vector<std::string> expected = {data + " " + field, (alias ? "affix_2/ " : "dp:met ") + field + " " + data,
                                               (alias ? "affix_1/ affix_2/ " : "dp:tek dp:met ") + field + " " + data,
                                               data + " " + field + (alias ? " /suffix_1" : " is:test")};
    for (size_t i = 0; i < words.size(); ++i) {
      const auto& word = words[i];
      const auto analysis = h.analyze(word);
      const auto stems = h.stem(word);
      const auto from_analysis = h.stem(analysis);
      auto normalized = [](const std::string& s) {
        auto start = s.find_first_not_of(' ');
        auto end = s.find_last_not_of(' ');
        return start == std::string::npos ? std::string() : s.substr(start, end - start + 1);
      };
      // Explicit stem metadata keeps its existing field position.
      std::string want = expected[i];
      if (root == "café") {
        want = (i == 1 ? "dp:met " : i == 2 ? "dp:tek dp:met " : "") + field + " " + data;
        if (i == 3)
          want += " is:test";
      }
      if (!h.spell(word) || analysis.size() != 1 || normalized(analysis[0]) != want || stems != std::vector<std::string>{stem} ||
          from_analysis != stems) {
        fprintf(stderr, "FAIL: %s %s (wanted analysis '%s', stem '%s')\n", fixture.c_str(), word.c_str(), want.c_str(),
                stem.c_str());
        for (const auto& a : analysis)
          fprintf(stderr, "  analyze: [%s]\n", a.c_str());
        for (const auto& s : stems)
          fprintf(stderr, "  stem: [%s]\n", s.c_str());
        ++failures;
      }
    }
    if (h.spell(root + "_invalid_"))
      ++failures;
  };
  run("alias3", "ouro", "ouro", "[stem_1]");
  run("gh1055", "élan", "élan", "po:noun xx:étiquette");
  run("gh1055", "café", "boisson", "po:noun xx:étiquette");
  return failures ? 1 : 0;
}
