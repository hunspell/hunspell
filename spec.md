# Java Hunspell Port – User-Facing API Specification (v1)

## 1) Goals

This document specifies the **public Java API** for a dependency-free Hunspell port that is intended to pass this repository’s existing test corpus (especially `tests/*.dic` driven by `tests/test.sh`).

Primary goals for the first milestone:

- Preserve core Hunspell behavior for **spelling and suggestions** first.
- Offer a Java-idiomatic API while retaining straightforward mapping from the C/C++ API where needed.
- Avoid external dependencies (JDK only).
- Enable test parity with existing `.good/.wrong/.sug` expectations first.

Deferred (explicitly out of scope for the first cut):

- Morphological analysis (`analyze`)
- Stemming (`stem`, `stem2`)
- Morphological generation (`generate`, `generate2`)
- Runtime dictionary mutation (`add/remove/add_with_*`)
- 100% binary/API compatibility with the C ABI
- JNI bindings
- Backward-compatibility promises before `1.0`

---

## 2) Repository Observations Driving This API

From this repo:

- The C API in `src/hunspell/hunspell.h` exposes many operations; for v1 we focus on create/destroy, spell, suggest, suffix_suggest, and dictionary loading.
- The C++ API in `src/hunspell/hunspell.hxx` adds richer semantics: optional spell info/root output, vector-based return types, dict encoding, wordchars, version/lang metadata.
- `tests/test.sh` validates the behavior surface through:
  - good-word acceptance (`hunspell -l`)
  - bad-word rejection (`hunspell -G`)
  - suggestion ranking/content

Therefore, the first Java API must expose spelling and suggestion features clearly and deterministically, and postpone morphology/mutation APIs until after a green baseline.

---

## 3) Package and Naming

Proposed package root:

- `org.hunspell`

Primary public types:

- `Hunspell` (main engine instance)
- `Hunspell.Builder` (construction/configuration)
- `SpellResult` (rich spell check output)
- `DictionaryInfo` (encoding/version/language metadata)
- `HunspellException` and subclasses (error model)

## 4) Lifecycle & Construction

### 4.1 Main Constructor Pattern

Use builder-based construction to keep options extensible:

```java
Hunspell hunspell = Hunspell.builder()
    .affix(Path.of("en_US.aff"))
    .dictionary(Path.of("en_US.dic"))
    .addDictionary(Path.of("custom.dic"))
    .build();
```

### 4.2 Builder API

```java
public static Hunspell.Builder builder();

public static final class Builder {
    public Builder affix(Path affPath);
    public Builder dictionary(Path dicPath);               // primary dictionary
    public Builder addDictionary(Path extraDicPath);       // equivalent to add_dic
    public Builder key(String key);                        // for encrypted dictionaries parity
    public Builder strictAffixParsing(boolean strict);     // default true
    public Builder maxSuggestions(int max);                // default aligns with Hunspell defaults
    public Hunspell build();
}
```

### 4.3 Resource Handling

`Hunspell` should implement `AutoCloseable`:

```java
try (Hunspell h = Hunspell.builder()...build()) {
   ...
}
```

Even if GC can reclaim memory, explicit close is useful for large dictionaries and deterministic cleanup.

---

## 5) Core Spellcheck API

### 5.1 Fast Boolean Check

```java
public boolean spell(String word);
```

Semantics: equivalent to C/C++ `spell`, returning accepted/rejected.

### 5.2 Rich Spell Result

```java
public SpellResult check(String word);
```

`SpellResult`:

```java
public final class SpellResult {
    public boolean correct();
    public boolean compound();          // maps SPELL_COMPOUND info bit
    public boolean forbidden();         // maps SPELL_FORBIDDEN info bit
    public Optional<String> root();     // root/stem if available
}
```

Reasoning: tests include root expectations; making root part of structured output avoids side-channel APIs.

---

## 6) Suggestion APIs

```java
public List<String> suggest(String misspelledWord);
public List<String> suffixSuggest(String rootWord);
```

Behavior notes:

- Return list order is meaningful (must preserve ranking for `.sug` compatibility).
- Return immutable lists to avoid accidental mutation.
- Empty list means no suggestions.

---

## 7) Metadata & Configuration Introspection

```java
public DictionaryInfo info();

public final class DictionaryInfo {
    public String encoding();
    public String version();
    public int languageNumber();
    public String wordCharacters();
}
```

These map to existing C++ getters (`get_dic_encoding`, `get_version`, `get_langnum`, `get_wordchars`).

---

## 8) Error Model

Base exception:

```java
public class HunspellException extends RuntimeException { ... }
```

Planned subtypes:

- `HunspellParseException` (aff/dic parse errors)
- `HunspellIoException` (I/O failures)
- `HunspellStateException` (illegal lifecycle/state)

Builder should fail fast on missing/invalid inputs.

---

## 9) Threading & Performance Contract

Initial contract:

- `Hunspell` instances are **not guaranteed thread-safe** in v1.
- Favor one instance per thread in early versions until we lock down concurrency guarantees.

## 10) Immediate CLI/Test Harness Plan (move earlier)

To reuse existing tests in this repository while implementing Java:

1. Scaffold the Java project skeleton immediately (module/package/layout + build script).
2. Implement a minimal CLI (`org.hunspell.cli.HunspellTool`) that mimics the subset of flags used by `tests/test.sh`:
   - `-l`, `-G`, `-a`, `-d`, `-i`
3. Add a tiny, very simple passing test first:
   - dictionary with no affixes (or mocked parser path)
   - exact verbatim dictionary hit should be accepted by `spell`
4. Add a test runner script that points `HUNSPELL` env var to the Java launch command for this reduced subset.
5. Expand progressively to existing `.good/.wrong/.sug` suites.

This creates a fast feedback loop before attempting full Hunspell parity.

---

## 11) Minimal v1 Public API (Proposed Freeze for First Implementation)

```java
package org.hunspell;

public interface Hunspell extends AutoCloseable {
    static Builder builder() { ... }

    boolean spell(String word);
    SpellResult check(String word);

    List<String> suggest(String word);
    List<String> suffixSuggest(String rootWord);
    int addDictionary(Path dicPath); // load extra .dic files only

    DictionaryInfo info();

    @Override
    void close();

    interface Builder {
        Builder affix(Path affPath);
        Builder dictionary(Path dicPath);
        Builder addDictionary(Path dicPath);
        Builder key(String key);
        Builder strictAffixParsing(boolean strict);
        Builder maxSuggestions(int max);
        Hunspell build();
    }
}
```

## 12) Implementation Phases Linked to Test Value

- **Phase 1**: parser + spell acceptance/rejection (`.good/.wrong`).
- **Phase 2**: suggestion engine parity (`.sug`).
- **Phase 3**: advanced compound/UTF and edge-case suites.
- **Phase 4**: root extraction support.
- **Phase 5**: morphology (`analyze/stem/generate`) and runtime mutation APIs.

---

## 13) Open Questions to Resolve Before Coding

1. **Interface vs class for `Hunspell`**: interface enables alternate engines; class simplifies implementation.
2. **Suffix suggestions in v1**: include now for parity, or defer until core suggest ranking is stable?
3. **Strictness defaults**: strict parsing gives safer behavior; lenient mode may help with malformed real-world dictionaries.
4. **JPMS modules**: optional for v1; easy to add once package structure stabilizes.

---

## 14) Proposed Next Step

Immediately scaffold the Java module layout and land a trivial passing test (verbatim dictionary hit), then wire the reduced CLI path (`-l/-G/-a/-d/-i`) to begin running selected existing tests.

---

## 15) Milestone Summary

### Milestone A — Bootstrap Java engine and harness
**Implemented**
- Java module scaffolding and CLI/test harness wiring.
- Minimal spell hit/miss behavior and smoke test execution path.

**Contribution**
- Established a runnable baseline for iterative parity work.

### Milestone B — Core affix algorithm port (`PFX`/`SFX` + conditions)
**Implemented**
- Parsing for core `.aff` directives (`SET`, `WORDCHARS`, `PFX`, `SFX`).
- Strip/add/condition evaluation and prefix+suffix cross-product generation.

**Contribution**
- Enabled first meaningful parity with Hunspell-derived forms (e.g., `condition` fixture behaviors).

### Milestone C — Charset-aware corpus handling and slash parsing fixes
**Implemented**
- Charset detection from `.aff` `SET` and charset-aligned `.dic` decoding.
- Bare slash (`/`) dictionary stem parsing support.
- Corpus-wide assertions for `condition`, `condition_utf`, and `slash` fixtures.

**Contribution**
- Unblocked UTF fixture stability and edge-case dictionary token parity.

### Milestone D — Case/punctuation normalization for realistic spell surface
**Implemented**
- Title-case and uppercase normalization paths for spell checks.
- Trailing-dot normalization (`...`) before lookup.
- Base fixture subset tests for affix/case/punctuation acceptance and rejection.

**Contribution**
- Improved user-facing spell behavior compatibility and expanded fixture parity (`base` subset).

### Milestone E — Root reporting in `check()` for generated forms (current session)
**Implemented**
- Root index tracking for stems and generated forms.
- `check(word)` now returns `SpellResult.root()` when a known root is available.
- Added tests for root reporting from both direct dictionary stems and affix-generated forms.

**Contribution**
- Advances API parity with the specified rich spell result model (root visibility), not just boolean spell outcomes.

### Milestone F — Manager-class refactor + FLAG formats and continuation classes (current session)
**Implemented**
- Split `SimpleHunspell` into package-private `AffixManager` (mirroring `affixmgr.cxx`) and
  `HashManager` (mirroring `hashmgr.cxx`); the public `SimpleHunspell` is now a thin façade
  playing the `Hunspell`/`HunspellImpl` role from `hunspell.cxx`.
- Replaced eager affix expansion with lookup-time recursion that mirrors the C++ control flow:
  `prefix_check` → `suffix_check` → `suffix_check_twosfx` → `prefix_check_twosfx`.
- `FLAG long`, `FLAG num`, and `FLAG UTF-8` decoding for both per-word stem flag vectors and
  per-rule continuation classes (`SFX A 0 s/123 .`).
- Two-level continuation-class suffix lookup driven by a parsed set of "flags appearing in
  some rule's contclass", matching how `AffixMgr` walks `contclasses[]` to enable chains like
  `foosbar` / `unfoosbar`.

**Contribution**
- Establishes the manager-class architecture target named in `plan.md` and unlocks parity for
  the `affixes`, `flag`, `flaglong`, `flagnum`, `flagutf8`, and `base_utf` ported subsets.

### Milestone G — Phase 1 closure: IGNORE, NEEDAFFIX, FORBIDDENWORD, BREAK (current session)
**Implemented**
- `IGNORE` directive: characters are removed from stored stems, affix strip/append, and
  spell-time input so the lookup pipeline operates in the same "ignore-stripped" space as
  C++ `AffixMgr::remove_ignored_chars`. `HashManager.load` now accepts a normalizer applied
  per stem.
- `NEEDAFFIX` flag: direct stem matches flagged NEEDAFFIX are skipped so the lookup falls
  through to affix derivation, while affix-derived forms satisfy the requirement.
- `FORBIDDENWORD` flag: the lookup path is refactored around a tri-state `LookupResult` so
  a forbidden surface form short-circuits case-variant and affix-derivation fallbacks,
  while non-forbidden homonyms still accept the word.
- `BREAK` directive: `HunspellImpl::spell_break` is mirrored as a bounded-depth recursive
  split with `^`/`$` anchored patterns, enabling hyphen/en-dash recursive acceptance while
  delegating each split half back through the full spell pipeline.

**Contribution**
- Closes Phase 1 of `plan.md` by shipping parity for the four remaining directives that
  the Phase 1 corpora exercise, taking the Java port from 10 to 14 ported `.good/.wrong`
  subsets with no regressions.

### Current parity snapshot
- Original C++ target corpus: **140 tests**.
- Ported/passing Java fixture coverage (meaningful subsets or full-file assertions where implemented):
  - `condition.dic`
  - `condition_utf.dic`
  - `slash.dic`
  - `base.dic`
  - `affixes.dic`
  - `flag.dic`
  - `flaglong.dic`
  - `flagnum.dic`
  - `flagutf8.dic`
  - `base_utf.dic`
  - `ignore.dic`
  - `needaffix.dic`
  - `forbiddenword.dic`
  - `break.dic`

---
