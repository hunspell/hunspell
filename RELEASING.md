# Releasing Hunspell

Two version numbers must be updated, independently:

1. **Package version** in `configure.ac` (`AC_INIT`).
2. **Libtool `version-info` triple** `current:revision:age` in
   `src/hunspell/Makefile.am`.

## libtool `version-info` rules

Apply in order, immediately before tagging:

1. Source changed since last release: `r += 1`.
2. Public interface added, removed, or changed: `c += 1; r = 0`.
3. Public interface added (since last release): `a += 1`.
4. Public interface removed or signature changed: `a = 0`.

Linux SONAME suffix is `current - age`. So pure additions keep the
SONAME stable; removals or signature changes increase it.

`tests/abi-check.sh` (run by `make check`) catches the third/fourth
case. After a deliberate ABI break, run `tests/abi-check.sh update`
and commit the new baseline.

## Checklist

1. Bump `AC_INIT` version in `configure.ac`, and the same version in
   `msvc/config.h` and `msvc/Hunspell.rc`. Rerun `configure` so the
   generated `src/hunspell/hunversion.h`, which is also committed, picks
   it up.
2. Update `-version-info` in `src/hunspell/Makefile.am`.
3. Add a `NEWS` entry.
4. `make check` (and `make dist`, build it from a clean dir).
5. Commit, `git tag vX.Y.Z`, push, attach the `make dist` tarball to
   the GitHub release.

## Worked example: 1.7.2 -> 1.7.3

Since v1.7.2: `add_with_flags` and a few macros added; nothing
removed or signature-changed.

Starting from `0:1:0`: rule 2 gives `1:0:0`, rule 3 gives `1:0:1`.
SONAME stays `libhunspell-1.7.so.0`, so it's a drop-in for 1.7.2.
