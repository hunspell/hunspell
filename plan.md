# Java Hunspell Port Plan (starting from `spec.md` item 12)

## Scope and intent
This plan expands the implementation phases defined in `spec.md` §12 into concrete milestones that preserve **C++ Hunspell code organization and algorithm behavior** while incrementally improving Java test parity.

Status legend:
- ✅ Completed
- 🟡 In progress
- ⬜ Not started

## Architecture alignment target (C++ -> Java)
To keep organization and algorithms close to the original implementation, split `SimpleHunspell` into components mirroring `src/hunspell/*.cxx` responsibilities:

- `HunspellEngine` (facade) ↔ `hunspell.cxx`
- `AffixManager` (PFX/SFX parsing + condition matching + cross-product) ↔ `affixmgr.cxx`
- `HashManager` (dictionary loading, flag decoding, entry/morph storage) ↔ `hashmgr.cxx`
- `SuggestManager` (REP/MAP/PHONE/ngram ordering) ↔ `suggestmgr.cxx`, `replist.cxx`, `phonet.cxx`
- `CsUtil` (casing, Unicode/encoding conversions, token handling) ↔ `csutil.cxx`, `utf_info.hxx`
- CLI adapter package ↔ `src/tools/hunspell.cxx` behavior subset

Current state is intentionally transitional: most logic is in `SimpleHunspell` and must be extracted in phases below.

---

## Phase 1: parser + spell acceptance/rejection parity (`.good/.wrong`)
**Status: 🟡 In progress**

### Workstreams
1. **Affix parser parity hardening**
   - Complete parsing coverage for key directives needed by base acceptance suites (flags, casing directives, compounding prerequisites).
   - Preserve rule application order and condition semantics as in `affixmgr`.
2. **Dictionary ingestion parity**
   - Handle escaped slashes, counts, comments, morphological fields, and flags using C++-equivalent tokenization logic.
3. **Spell pipeline equivalence**
   - Ensure lookup normalization (case handling, trailing punctuation handling) matches `hunspell::spell` flow.
4. **Refactor for parity readability**
   - Split parser/checker into dedicated classes before adding many more features.

### Exit criteria
- Java passes ported `.good/.wrong` subsets for at least: `condition`, `condition_utf`, `base`, `slash`.
- No regressions in existing Java spell tests.

### Current progress evidence
- Existing Java tests already validate the above four corpora as ported subsets and pass.
- Remaining gap: broad `.good/.wrong` matrix across the full test corpus and closer structural separation into manager classes.

---

## Phase 2: suggestion engine parity (`.sug`)
**Status: ⬜ Not started (parity-grade implementation)**

### Workstreams
1. Replace current Levenshtein-only ranking with Hunspell-like staged suggestion pipeline:
   - edits/transpositions
   - REP table replacements
   - MAP equivalence classes
   - PHONE/phonetic suggestions
   - ngram scoring and weighting/order rules
2. Implement suggestion filtering flags (`NOSUGGEST`, forbidden interactions, casing normalization).
3. Preserve deterministic order compatible with `.sug` golden expectations.

### Exit criteria
- Port first suggestion suites (`sug`, `sug2`, `map`, `rep`) into Java tests and achieve passing parity for ranked outputs.

### Gap note
- Current `suggest()` is intentionally simplified and does not mirror `suggestmgr` algorithmic stages.

---

## Phase 3: advanced compound/UTF edge-case suites
**Status: ⬜ Not started**

### Workstreams
1. Implement compound controls equivalent to C++ (`COMPOUNDRULE`, `ONLYINCOMPOUND`, duplicates/triple checks, pattern checks).
2. Expand UTF and locale-sensitive handling to match `csutil` behavior (including language-specific edge cases where feasible).
3. Add edge-case parsers and runtime guards used in regression tests.

### Exit criteria
- Port and pass representative compound suites (e.g., `compoundrule*`, `checkcompound*`, `onlyincompound*`) and UTF-focused suites beyond current condition tests.

---

## Phase 4: root extraction support
**Status: ✅ Completed (initial parity scope)**

### Workstreams
1. Ensure `check(word).root()` maps generated forms back to stem.
2. Retain root provenance for direct entries and affix-generated entries.

### Exit criteria
- Passing tests for direct and generated-word root extraction.

### Gap note
- Future improvement: richer morphology/root metadata parity beyond plain stem string.

---

## Phase 5: morphology + runtime mutation APIs
**Status: ⬜ Not started**

### Workstreams
1. Public API expansion for `analyze/stem/generate/generate2` equivalents.
2. Runtime dictionary mutation APIs (`add/remove/add_with_affix` style behaviors).
3. Internal storage changes to preserve morphology fields and aliases in C++-compatible form.

### Exit criteria
- API and behavior parity demonstrated with newly ported morphology-focused tests.

---

## Progress tracking rule
Progress updates must be recorded directly under the corresponding phase section (status + current progress evidence), not in a separate global log.
