// Regression test for stack-buffer-overflow in Hunzip::getline.
//
// A crafted .aff.hz that decompresses to BUFSIZE printable bytes with no
// EOL caused linebuf[BUFSIZE] = '\0' to write one byte past the end of the
// 65 KiB stack buffer. Reproducible through the public Hunspell constructor
// because FileMgr falls through to Hunzip when only the .aff.hz exists.
//
// Run under ASan/UBSan: this program must complete without a sanitizer error.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "../src/hunspell/hunspell.hxx"

static int write_poc(const char* path) {
    // Header: magic "hz0" + 16-bit big-endian record count (=2)
    // Code 1: c=('a','a'), len=1 bit, bits "0"
    // Code 2: c=('b','b'), len=1 bit, bits "1"
    // 8192 zero bytes -> 65536 zero bits -> 32768 "aa" pairs = 65536 'a' bytes.
    static const unsigned char hdr[] = {
        'h','z','0',
        0x00, 0x02,
        'a','a', 0x01, 0x00,
        'b','b', 0x01, 0x80,
    };
    FILE* f = fopen(path, "wb");
    if (!f) return 1;
    if (fwrite(hdr, 1, sizeof hdr, f) != sizeof hdr) { fclose(f); return 1; }
    static const unsigned char zero[1024] = {0};
    for (int i = 0; i < 8; ++i)
        if (fwrite(zero, 1, sizeof zero, f) != sizeof zero) { fclose(f); return 1; }
    fclose(f);
    return 0;
}

int main(int argc, char** argv) {
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir || !*tmpdir) tmpdir = "/tmp";

    std::string base(tmpdir);
    base += "/hunspell-gh-hunzip-overflow";
    std::string aff = base + ".aff";        // intentionally absent
    std::string aff_hz = base + ".aff.hz";  // crafted
    std::string dic = base + ".dic";        // empty stub is fine

    // Make sure no stale .aff is around so FileMgr falls through to .hz.
    remove(aff.c_str());

    if (write_poc(aff_hz.c_str()) != 0) {
        fprintf(stderr, "FAIL: cannot write %s\n", aff_hz.c_str());
        return 1;
    }
    FILE* d = fopen(dic.c_str(), "w");
    if (!d) { fprintf(stderr, "FAIL: cannot write %s\n", dic.c_str()); return 1; }
    fclose(d);

    // The constructor must not trigger a sanitizer error. Pre-fix, ASan reports
    //   stack-buffer-overflow ... WRITE of size 1 ... in Hunzip::getline
    Hunspell h(aff.c_str(), dic.c_str());
    h.spell(std::string("test"));

    remove(aff_hz.c_str());
    remove(dic.c_str());
    (void)argc; (void)argv;
    return 0;
}
