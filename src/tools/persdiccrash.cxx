// Test for use-after-free in add_with_affix when add_word fails
#include "../hunspell/hunspell.hxx"
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <aff> <dic>\n", argv[0]);
        return 1;
    }

    Hunspell dict(argv[1], argv[2]);

    // A word longer than 65535 bytes causes add_word to fail.
    // If the failure frees flags that add_hidden_capitalized_word
    // then reads, ASan will detect a use-after-free.
    // ALLCAP ensures add_hidden_capitalized_word is entered.
    std::string longword(70000, 'A');
    dict.add_with_affix(longword, "example");

    return 0;
}
