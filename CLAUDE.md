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

`C/cui18n.{h,cpp}` is a thin wrapper: an opaque `UI18N_CTranslationEngine*` that is really a
`TranslationEngine*`, cast via the `cast()` macro. `UI18N::Internal` (a `friend` of `TranslationEngine`) exists
solely to let the C layer reach the private `cAPITmpResultStorage` vector, which owns the returned `const char*`
buffers — they stay alive for the life of the engine and grow without bound, by design.

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

`get()` copies the `Variable` out of the map by value and mutates the copy, so the stored translation is never
consumed. Note it uses `operator[]` deliberately (see the comment in the source): unknown IDs insert an empty
entry and return `""` rather than throwing. The library is exception-free and nearly everything is `noexcept`;
`parseConfig`/`parseTranslations` are the exceptions since ryml may throw.

### Translation file format

`init(directory, defaultLocale)` reads `<directory>/ui18n-config.yaml`, then scans the directory (non-recursively)
for `*.yaml`/`*.yml` whose stem matches a language code. Both `en_US.yaml` and `en-US.yaml` spellings are accepted.
Files that don't match a known code are silently ignored. A missing/unparseable config returns
`UI18N_INIT_RESULT_INVALID_CONFIG` and aborts; a bad translation file downgrades the overall result to
`UI18N_INIT_RESULT_INVALID_TRANSLATION` but parsing of other files continues.

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
  matches portably across GCC/Clang/MSVC.

Frequent "Update ryml" commits come from the automated `update.yml` workflow bumping submodules; rapidyaml API
churn is the usual cause of build breakage here.

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
