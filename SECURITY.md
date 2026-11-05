# Security Policy

## Supported versions

Only the 1.7 series is maintained. Fixes go to `master`. If you are
running something older, the answer to a security report is to
upgrade.

## What is in scope

Memory safety problems reachable from ordinary use: a crash, or a read
or write outside a buffer, when checking or suggesting for words taken
from a document, against a dictionary installed the normal way.

## What is out of scope

1. A crafted dictionary. Dictionaries are installed data, from a
   distribution package, an extension the user chose, or the embedding
   application's own download service, and an extension can already
   run code as the user. A bug that needs a crafted dictionary is
   fixed as an ordinary bug, with no advisory and no CVE. If a CVE is
   assigned for one anyway, we will ask for it to be marked disputed.
2. Wrong spelling results, missing words, and bad suggestions. Those
   are dictionary questions and belong to the dictionary project for
   the language concerned.
3. High memory use or slow checking on input built to be pathological.
4. Bugs in the applications that embed Hunspell. Report those to the
   application.

One report per root cause. A variant of an issue already reported, or
of a fix under review, earns no separate credit, and a finding against
code that is in no release is fixed on master without an advisory.

Hunspell is fuzzed continuously by oss-fuzz, which has its own tracker
and its own disclosure deadline. Findings that arrive that way are
already being handled and do not need reporting here as well.

## How to report

Use GitHub's private reporting: the Security tab of this repository,
then "Report a vulnerability". That keeps the report private until
there is a fix.

If that does not work for you, mail caolanm@gmail.com.

A report is most useful with the input that triggers it, the version
or commit you tested, and the sanitizer output if you have it. A
reproducer file is worth more than a description of one.

## What happens next

Hunspell has one active maintainer, and there is no fixed timetable.

Most findings are fixed in a public commit once they are understood,
because the fix is usually a bounds check whose absence is visible in
the code anyway. A finding serious enough to need handling differently
is handled case by case.

Please allow 90 days before disclosing publicly.

Reporters are credited unless they ask not to be.
