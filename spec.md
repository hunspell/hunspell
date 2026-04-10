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

## 15) Work Log Summary

### Session Date
- 2026-04-10

### Implementation Progress in This Session
- Improved `.dic` parsing in the Java engine so escaped slashes (`\/`) are decoded as literal `/` in dictionary entries.
- Updated parsing to stop stem extraction at unescaped flag separators (`/`) and whitespace (so entries like `created/U<TAB>st:created` load as stem `created`).
- Kept behavior compatible with the current minimal v1 scope (still no affix-rule generation; this session only improves raw dictionary stem loading fidelity).

### Tests Added/Updated
- Added Java unit coverage for escaped slash entries (mapped from the C++ corpus intent in `tests/slash.dic` / `tests/slash.good`).
- Added Java unit coverage for dictionary entries containing flags + morphology fields on a single line.
- Added CLI coverage for `-d` dictionary base-path resolution without `.dic` extension.

### Passing Test Count (Session-over-Session)
- Previous session: **8 passing Java tests**
- Current session: **11 passing Java tests**
- Net change: **+3 tests passing** (no regressions)

### Test Parity vs Original Implementation
- Original C++ suite target listed in `tests/Makefile.am`: **140 tests**.
- Java port passing tests in this session: **11 focused unit tests**, including one direct port of slash dictionary behavior and two API/CLI behavior checks aligned with the v1 subset.
- Parity status: **partial / early bootstrap stage**; Java does not yet execute the full C++ corpus and does not yet implement affix expansion/suggestion parity.

### Ported C++-Corpus Behaviors Currently Passing in Java
- ✅ `slash` dictionary escaped-slash acceptance behavior (from `tests/slash.dic` + expected accepted tokens in `tests/slash.good`) is now covered and passing in Java tests.

---

## Session Work Log

### Session 1 — April 10, 2026

**Summary**
- Added initial Java API scaffolding under `java/src/main/java/org/hunspell` matching the proposed v1 public surface (`Hunspell`, `Builder`, `SpellResult`, `DictionaryInfo`, exceptions).
- Added a temporary bootstrap implementation (`SimpleHunspell`) that can load `.dic` word lists and perform exact-hit spell checks.
- Added reduced CLI (`org.hunspell.cli.HunspellTool`) with starter support for `-d`, `-l`, `-G`, and `-a` to begin wiring test harness workflows.
- Added a first smoke test script (`java/tests/session1_smoke.sh`) validating one minimal `-l` flow end-to-end.

**Work-log test progression requirement**
- Previous session passing tests: **0** (baseline).
- Current session passing tests: **1** (`session1_smoke.sh`).
- Status: **Improved** (current > previous).

**Parity with original implementation**
- Current parity status: **very early bootstrap only**.
- Original implementation (`hunspell` C/C++) supports broad affix logic, rich suggestions, and full `tests/test.sh` suite coverage.
- Java port currently demonstrates only minimal dictionary-hit behavior and does **not yet** claim parity on existing `.good/.wrong/.sug` corpora.

### Session 2 — April 10, 2026

**Summary**
- Added Maven project configuration at `java/pom.xml` so the Java scaffold has a standard build/test entrypoint.
- Added JUnit 5 bootstrap tests in `java/src/test/java/org/hunspell/HunspellBootstrapTest.java` for exact-hit acceptance and rejection of missing words.
- Kept the existing smoke script path to validate CLI behavior end-to-end alongside Maven unit tests.

**Work-log test progression requirement**
- Previous session passing tests: **1**.
- Current session passing tests: **3** (2 JUnit tests + 1 smoke script).
- Status: **Improved** (current > previous).

**Parity with original implementation**
- Parity remains **partial/early**.
- Improvements this session add repeatable Java-side build/test infrastructure, but still do not cover full Hunspell affix behavior or the complete existing `.good/.wrong/.sug` parity surface.

---

## 15) Work Log

### Session 2026-04-10 (Continue the work)

- Expanded Java unit coverage from the prior 2 JUnit tests to 8 JUnit tests, and kept the existing CLI smoke script green.
- Added test coverage for:
  - extra dictionary loading (`addDictionary`)
  - `suggest` limit and deterministic ordering
  - `suffixSuggest` root-prefix behavior
  - `check` result shape
  - CLI `-l` and `-G` output filtering behavior
- Updated implementation for deterministic suggestion ordering (distance, then lexical tie-break).

### Session 2026-04-10 (Address review feedback)

- Clarified repository workflow instructions by updating `AGENTS.md` to explicitly require updating `tests.md` whenever C++ tests are ported and passing in Java.
- Updated `tests.md` checklist progress by marking `slash.dic` as ported/passing in Java (subset behavior coverage).
- Added a new Java unit test to verify dictionary comment lines are ignored during load.

**Work-log test progression requirement**
- Previous session passing tests: **11**.
- Current session passing tests: **12**.
- Status: **Improved** (current > previous), with no regressions.

**Parity with original implementation**
- Original implementation target remains **140 C++ tests**.
- Java parity remains **partial**, but now explicitly tracked in `tests.md` with `slash.dic` marked as ported/passing subset behavior.
- Updated implementation for `suffixSuggest` to return words that start with the supplied root.

**Pass count versus previous session:**
- Previous session: 2 passing JUnit tests (+ 1 shell smoke check).
- This session: 8 passing JUnit tests (+ 1 shell smoke check).
- Net: **+6 additional passing JUnit tests**.

**Test parity versus original implementation:**
- Still **far from parity** with the original Hunspell C/C++ implementation and the full `tests/test.sh` corpus.
- Current Java port validates only a narrow bootstrap subset (basic dictionary hit/miss, simple suggestions, and minimal CLI filtering modes).
- Suggestion quality/ranking, affix logic, compound handling, encoding edge cases, and morphology remain not yet equivalent.
## Work Log Summary (2026-04-10)

- Attempted to run the C++ test suite with `make check`, but no generated Makefile was present yet (`No rule to make target 'check'`).
- Attempted to bootstrap the autotools build with `autoreconf -fi`; this failed because `autopoint` is not installed in the environment.
- Current test parity status versus the original C++ implementation: **not yet measurable in this environment** because the native C++ harness could not be built/run.
- Added a repository-level checklist file (`tests.md`) enumerating every test from `tests/Makefile.am` `TESTS` for tracking execution and Java-port progress.
