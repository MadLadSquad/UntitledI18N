# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

UntitledI18N is a standalone cross-platform i18n library for C and C++ (MadLadSquad, MIT). It provides a
`TranslationEngine` that loads YAML translation files and resolves message IDs into interpolated strings.

The entire implementation is four files: `Common.h`, `UI18N.hpp`/`UI18N.cpp`, and the C wrapper in `C/`. Everything
else is submodules (`rapidyaml`, `parallel-hashmap`), CMake glue, and CI.

This repo is normally consumed as a git submodule of
[UntitledImGuiFramework](https://github.com/MadLadSquad/UntitledImGuiFramework) at
`Framework/Modules/i18n/ThirdParty/UntitledI18N` (see "Embedding" below), but it builds and installs on its own.

End-user documentation (install instructions, translation format reference) lives on the
[wiki](https://github.com/MadLadSquad/UntitledI18N/wiki), not in this repo.

## Building

```bash
bash ci.sh              # exactly what CI runs: cmake -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_VARIANT_VENDOR=ON, then MSBuild or make
```

Manually:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_VARIANT_VENDOR=ON
make -j$(nproc)
```

There is no test suite and no linter. CI (`.github/workflows/CI.yaml`) only verifies that the library compiles on
Linux, Windows (MSBuild) and macOS. Verify changes by building; a translation-parsing change is best checked by
writing a throwaway `main.cpp` against the built library.

### CMake options

- `BUILD_VARIANT_STATIC` — builds a `STATIC` instead of `SHARED` library.
- `USE_PRECONFIGURED_YAML` — when `ON`, the parent project has already added rapidyaml and provides its include
  dirs via `RYML_INCLUDE_DIRS_T`; when `OFF`, `rapidyaml/` is added as a subdirectory here.
- `UIMGUI_INSTALL` — gates all `install()` rules (headers under `include/UntitledI18N`, libs under `lib64/`,
  pkg-config file under `lib/pkgconfig/`).

`cmake/FindYamlCpp.cmake` is vestigial — the library parses YAML with rapidyaml, not yaml-cpp. (CI still installs
`yaml-cpp` on Linux/macOS; that too is leftover.)

### Compile-time defines

- `MLS_EXPORT_LIBRARY` + `MLS_LIB_COMPILE` — set by this CMakeLists when compiling the library; on Windows they
  turn `MLS_PUBLIC_API` into `dllexport`. Consumers set only `MLS_EXPORT_LIBRARY` (→ `dllimport`). Every public
  symbol must be annotated with `MLS_PUBLIC_API`.
- `UI18N_CUSTOM_STRING` + `UI18N_CUSTOM_STRING_INCLUDE` — replace `std::string` as the `ui18nstring` typedef
  (used by the framework to swap in its allocator-aware string). Defining the first without the second is a
  hard `#error`.
- `UIMGUI_I18N_MODULE_ENABLED` — exported `PUBLIC` by the target and re-exported through the `.pc` file's
  `compile_defs` variable, so downstream code can `#ifdef` on i18n availability.

Requires C++23 (`std::string::ends_with`, concepts) and C99.

## Architecture

### Layers

`Common.h` is the C ABI surface: the `UI18N_LanguageCodes` enum (~280 POSIX-style locale codes, terminated by
`UI18N_LANGUAGE_CODES_COUNT`), `UI18N_InitialisationResult`, and the `MLS_PUBLIC_API` export macro. It is included
by both the C++ header and the C header.

`UI18N.hpp`/`UI18N.cpp` hold `UI18N::TranslationEngine` plus the free functions `languageCodeToString` /
`stringToLanguageCode`. `LanguageCodesAsStrings` in `UI18N.cpp` **must stay 1:1 in order and length with the enum
in `Common.h`** — lookups index one array with the other's enum value.

`stringToLanguageCode` walks that array and compares character by character, folding case with an **ASCII-only**
`asciiToLower` rather than `std::tolower`: a locale code is pure ASCII, whereas `tolower` is undefined for negative
(non-ASCII UTF-8) bytes and answers to whatever `LC_CTYPE` the host program has set. It allocates nothing and bails
out at the first differing character, which matters because `init()` calls it once per file in the directory it
scans — the older form built two fresh lowercase strings for each of the 298 candidates on every lookup.

`C/cui18n.{h,cpp}` is a thin wrapper: an opaque `UI18N_CTranslationEngine*` that is really a
`TranslationEngine*`, cast via the `cast()` macro. `UI18N::Internal` (a `friend` of `TranslationEngine`) exists
solely to let the C layer reach the private `cAPITmpResultStorage`, which owns the returned `const char*`
buffers — they stay alive for the life of the engine and grow without bound, by design. It must stay a
**`std::deque`, never a `std::vector`**: vector growth relocates its elements, and a short string keeps its
characters inside the element itself (small-string optimisation), so every `const char*` handed out earlier would
dangle the moment the vector reallocates.

C callers can pass null anywhere, and `ui18nstring` cannot be built from a null pointer — libstdc++ answers that
with a `std::logic_error`, which in this library means `std::terminate`. So `UI18N_TranslationEngine_get` drops
any `UI18N_Pair` missing its key or its value rather than guessing at it, leaving the placeholder to its switch
default exactly as if the caller had not passed it, and skips null positional arguments the same way. A null
`engine` is the one input it answers with `nullptr`: the result buffer lives in the engine, so there is nowhere to
put a string. A null `id` is not an error — `get()` already reports an unknown id as `""`.

### Data model

`translations` is a `std::array<ui18nmap<ui18nstring, Variable>, UI18N_LANGUAGE_CODES_COUNT>` — one map per
locale, indexed directly by the enum, so locale switching is just an index change (`currentLocale`).

A `Variable` is `{ text, references }` where `references` maps each `{name}` placeholder found in the text to a
`Switch`. A `Switch` (`{ bExists, defaultValue, patterns }`) is only populated when the translation declared a
`switch:` block; otherwise `bExists` is false and the placeholder is a plain substitution.

Three substitution mechanisms, applied in this order by `get()`:
1. **Positional** — `{}` occurrences replaced left-to-right from `positionalArgs`.
2. **Engine variables** — pushed once via `pushVariable()`, stored in `variables`, checked first.
3. **Call-site args** — the `args` map, checked only if no engine variable matched that name.

Terms from `ui18n-config.yaml` are different: they are substituted **at parse time** into the translation text,
switch defaults, and switch results — never at `get()` time.

`get()` looks the id up with **`find()`, never `operator[]`**, and copies out only `text` — the one field it
mutates — so the stored translation is never consumed. `operator[]` would turn every lookup into a write: two
threads calling `get()` would race the moment either one missed, and a program resolving ids it does not have
would grow the map without bound. As written, `get()` is a pure read and is safe to call concurrently. An unknown
id, a null id, and an out-of-range `currentLocale` all return `""`. Every function in the library is `noexcept` —
see "Error handling" below.

Every container the resolution path consults is a hash map — `variables`, `args`, a `Switch`'s `patterns`, and
`terms` at parse time — and each is reached with `find()`. Scanning them, as this used to, is not merely
O(references × (variables + args)): a scan yields the map's own `value_type`, whose key is `const`, so binding it
to a `std::pair<ui18nstring, ui18nstring>` copied both strings on every match.
`getHandleReplaceWithVal` therefore takes the variable's name and value as two separate parameters — don't fold
them back into a pair.

`init()` clears `translations`, `terms` and `existingLocales` before loading, so calling it again switches
translation directories cleanly instead of merging them (`insert` keeps the *older* entry on a duplicate id, so a
merge would let stale translations win). Deliberately kept across a re-init: `variables`, which is pushed through
the public API rather than read from the directory, and `cAPITmpResultStorage`, which backs pointers the C API
promised would outlive individual calls. A failed re-init leaves the engine empty rather than still serving the
previous directory.

### Translation file format

`init(directory, defaultLocale)` reads `<directory>/ui18n-config.yaml`, then scans the directory (non-recursively)
for `*.yaml`/`*.yml` and resolves each stem through `stringToLanguageCode` — deliberately the same lookup the
public API uses, so a file name and a call always agree on which spellings are valid: case-insensitive, either
separator (`en_US.yaml`, `en-US.yml`, `en_us.yaml`). The extension is matched case-insensitively too
(`en_GB.YAML`, `en_GB.Yml`), so the whole file name is; a name is the user's to write, and on Windows and macOS
the same file may answer to several spellings. Only `.yaml`/`.yml` count — no other extension is read. Files that don't match a known code are silently ignored.
A missing or unreadable config returns `UI18N_INIT_RESULT_INVALID_CONFIG` and stops. An *empty* config is not an
error — it declares no terms, which is legal — so `loadFileToString` reports "could not open" separately from
"opened, no content"; only the first is fatal. An empty *translation* file is still
`UI18N_INIT_RESULT_INVALID_TRANSLATION`, since a file named after a locale promises translations. A translation
file that parses but is structurally wrong downgrades the overall result to `UI18N_INIT_RESULT_INVALID_TRANSLATION`
while parsing of the other files continues. A file that fails to *parse* is not recoverable — see "Errors" under
"rapidyaml integration gotchas".

Translation files are read in **binary mode**. In text mode Windows collapses each CRLF to one character, so fewer
characters arrive than `tellg()` reported and the tail of the buffer keeps its padding; the read is additionally
truncated to `gcount()` rather than to the reported size.

There is exactly one spelling of each code, and it is the one in `LanguageCodesAsStrings` — no alias table, no
deprecated enumerators. Japanese was renamed `jp_JP` → `ja_JP` (`ja` is the ISO 639-1 language code; `jp` is only
the country code) as a clean break: `jp_JP.yaml` files and code naming the `jp_JP` enumerator must be updated.
Renaming a code this way is a source-breaking change but not an ABI-breaking one, as long as the entry keeps its
position — the enum is numbered implicitly and `translations` is indexed by it, so reordering or inserting shifts
every locale after it.

`ui18n-config.yaml`:
```yaml
terms:
  - company-name: MadLadSquad
```

`<locale>.yaml`:
```yaml
translations:
  - id: greeting
    text: "Hello {name}, welcome to {company-name}! You have {} messages."
    switch:
      - var: name
        default: "stranger"
        cases:
          - case: admin
            result: "boss"
```
`terms` and the `switch`/`cases` blocks are **sequences of single-pair maps**, not plain maps — the `read_dict`
helper descends into each sequence element's child pair. Both `translations` and `switch`/`cases` must be
sequences or the node is rejected.

### rapidyaml integration gotchas

The `c4::yml` overloads at the bottom of `UI18N.cpp` carry non-obvious constraints, documented inline — preserve
them when touching that code:

- `read(ConstNodeRef const&, ui18nstring*)` is a plain `static` (internal-linkage) overload rather than an
  explicit specialisation, because newer rapidyaml routes the primary `read()` template through `ReadResult`.
  `static` keeps the symbol TU-local so it doesn't clash with the framework's own `read(std::string*)` when
  i18n is compiled into it.
- `read_dict` takes the container type directly instead of a template-template parameter, because
  `phmap::parallel_flat_hash_map` mixes type and non-type template parameters and no template-template form
  matches portably across GCC/Clang/MSVC. It copies each key with `memcpy` for exactly `key.len` bytes and
  `static_assert`s that the key type is string-like: a ryml key is a `csubstr`, a pointer and a length into the
  parse arena with **no terminator of its own**, so building a key from the bare pointer runs on past the end of
  the arena. (Instantiated with a `const char*` key, the version that did so read 246 bytes past a 256-byte
  arena under ASan.) A new key type that cannot be filled by length needs an explicit conversion here, which is
  what the assertion asks for.

**Errors: guard before you access, and never write recovery machinery.** ryml reports every error by calling a
callback that *must not return* (see `pfn_error_parse` in `rapidyaml/src/c4/yml/common.hpp`); the README names the
only three ways to implement one — an exception, `std::longjmp`, or `abort()`. There is no status-returning parse
API: `parse_in_arena` returns `void`/`Tree`, and `Parser` carries no error flag. `ReadResult` is a
*deserialization* result, not a parse one. So the way to stay error-free is to never reach the callback:

- Read through **`ConstNodeRef` + `find_child("x")`**, never `operator[]`, whenever the key may be absent —
  `find_child` returns an invalid ref for a missing key, which `keyValid` rejects, whereas const `operator[]`
  errors. Check `is_seq()` before iterating children, and `has_key()`/`has_val()` before `key()`/`val()` (the
  `read` overload at the bottom of `UI18N.cpp` does the latter). One malformed field then fails that field
  instead of the whole file.
- What this does **not** cover is a *syntactically* malformed document: `parse_in_arena` reaches the callback and
  the default handler aborts. That is a known and accepted limitation — `init`'s `INVALID_CONFIG`/
  `INVALID_TRANSLATION` results cover a missing or unreadable file, an empty translation file, and a file that
  parses but is structurally wrong, not a file that fails to parse. The framework accepts the same limitation for
  its own configs.
- Do **not** reintroduce a `setjmp`/`longjmp` guard, a custom error callback, or allocation bookkeeping to work
  around this. It has been tried; it costs far more complexity than the behaviour is worth, and jumping out of
  the parser is undefined behaviour the moment a non-trivial destructor is skipped.

Frequent "Update ryml" commits come from the automated `update.yml` workflow bumping submodules; rapidyaml API
churn is the usual cause of build breakage here.

## Error handling — no exceptions

**Never use `throw`, `try`/`catch`, or any API whose error path is an exception.** This is not a style
preference: UntitledImGuiFramework consumes this library, and its Emscripten builds have no exception support at
all, so a throw does not unwind — it calls `abort()` and takes the process down, and a `catch` written against it
is dead code. Every function here is additionally marked `noexcept`, where an escaping exception is
`std::terminate` on every platform.

Report errors by return value instead: an `InitialisationResult`, a `bool`, an empty string, or a null handle.
Keep new public API `noexcept`, and keep private helpers `noexcept` too — the annotation is what makes an
accidental throwing call a hard error rather than a silent hazard.

Standard-library APIs to avoid, with the replacement used in-tree:

- **`std::filesystem`'s throwing overloads** — `directory_iterator`, `directory_entry::is_directory`,
  `absolute`, and the iterator's `operator++` all throw `filesystem_error`. Every one of them has an overload
  taking a `std::error_code&`; `init` uses those exclusively, and steps the iterator with `it.increment(ec)`
  rather than `++it`.
- **`std::filesystem::path::string()`** — converts between encodings and throws when the conversion fails. Use
  `path::native()`, which is a plain accessor. `pathToASCII` in `UI18N.cpp` is the in-tree conversion: language
  code file names are pure ASCII, and anything else can never match a code, so it is rejected rather than
  converted.
- **`operator new`** — use `new(std::nothrow)` and null-check, as `UI18N_TranslationEngine_Construct` does.
- **`.at()` on containers** — use `operator[]` after checking bounds, or `find()`.
- **`std::stoi`/`std::stof`/`std::stod`** — use `std::from_chars` or the `strtol`/`strtod` family.
- **rapidyaml** — never throws with the default callbacks, but they abort, which is just as fatal. There is no
  error-returning parse API; the only defence is to guard node access so the callback is never reached. See
  "rapidyaml integration gotchas".

`std::bad_alloc` is the one exception (in both senses) that is out of scope: allocation is pervasive here —
`ui18nstring` alone allocates on nearly every line — so there is no meaningful boundary at which to handle it.
Allocations we make *directly*, on the other hand, still use the non-throwing forms above.

## Embedding in UntitledImGuiFramework

`Framework/cmake/SetupOptions.cmake` either `add_subdirectory`s this repo with `USE_PRECONFIGURED_YAML ON` (vendored
build) or resolves it via `pkg_check_modules(UntitledI18N REQUIRED UntitledI18N)` (system build), which is why the
`.pc` file and the `compile_defs` passthrough matter. `Framework/Modules/i18n/src/I18NModule.{hpp,cpp}` wraps the
engine in a static `UImGui::I18N` interface backed by a single engine instance owned by `UImGui::Modules`.

Public functions are annotated with `// UntitledImGuiFramework Event Safety - Any time` comments; keep that
annotation on new public API, since the framework's docs rely on it.

## Conventions

- Version in `CMakeLists.txt` is rewritten by `.github/workflows/release.yaml` from the pushed `v*` tag — don't
  bump it by hand.
- Loops use forward `goto` to labels (`exit_inner_loop:;`) for multi-level breaks; this is the established style
  here, not an accident.
- British spelling in identifiers (`InitialisationResult`, `parseVariablePatternMatching`).

## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).
