# Translations

This document describes the translation mechanism of the two components and how to extend it.

## Scope

Only the strings that reach the user are translated:

- **Overlay**: every string rendered on screen (frame titles and subtitles, list items, category headers, toggles, drawn hints and error messages).
- **Sysmodule**: every string sent in Ultrahand notifications (the attention title, the "monitoring started" message, the remaining-time messages and their time units).

What is **not** translated: log messages, debug/error sentinels (`Err#...`, `ERR#...`), IPC names, setting keys, and file names.

Supported languages: **English** (fallback) and **French**.

## Language detection

The language is the one configured on the console, read with the SET service:

```cpp
setInitialize();
i18n::detectSystemLanguage();
setExit();
```

- `setGetSystemLanguage()` returns the console `LanguageCode`;
- `setMakeLanguage()` converts it to the `SetLanguage` enum;
- French (including Canadian French) selects the French strings, every other language (or an error) falls back to English.

Detection happens once at startup:

- **Sysmodule**: in `main()`, before the IPC service and the monitor threads start (see `sysmodule/source/main.cpp`). To apply a console language change to the notifications, the sysmodule must be restarted.
- **Overlay**: in `main()`, each time the overlay is opened (see `overlay/src/main.cpp`). No restart is needed on the overlay side.

On the host (unit tests, no `__SWITCH__`), `detectSystemLanguage()` always falls back to English and the tests switch the language explicitly with `setLanguage()`.

## Mechanism

Each component has its own translation module in its own `i18n/` directory (the two builds do not share sources — same convention as the duplicated IPC command enum):

| Component | Header | Implementation |
|-----------|--------|----------------|
| Sysmodule | `sysmodule/source/i18n/i18n.h` | `sysmodule/source/i18n/i18n.cpp` |
| Overlay   | `overlay/src/i18n/i18n.h`      | `overlay/src/i18n/i18n.cpp`      |

Both modules expose the same API in the namespace `alefbet::pctrl::i18n`:

- `enum class Language { English, French }`
- `Language current()` — the active language.
- `void setLanguage(Language)` — override the language (used by the host tests).
- `void detectSystemLanguage()` — read the console language (SET service, target only).
- `const char* tr(const char* sourceText)` — the string in the active language. The argument is the **English source text**; it is returned unchanged when no translation exists.
- `std::string format(const char* sourceTemplate, const std::string& value)` — like `tr`, but replaces the single `%s` placeholder of the translated template (e.g. the remaining-time notification: `"%s left"` / `"Il reste %s"`).

### Translation tables (one file per language)

English is the source language: it has **no table**, the English strings live in the call sites themselves. Every other supported language has its own table file in the same `i18n/` directory, a `std::unordered_map<std::string, const char*>` (English source text -> translated text):

| Component | French table |
|-----------|--------------|
| Sysmodule | `sysmodule/source/i18n/i18n_fr.cpp` (`french()`) |
| Overlay   | `overlay/src/i18n/i18n_fr.cpp` (`french()`)      |

`tr()` looks the source text up in the active language's table and falls back to the source text itself when the language has no table (English) or the entry is missing (a stale or missing translation therefore degrades to English, never to an empty string). The tables are function-local statics, so they are initialized on first use (after the heap is up on the target). The SET-service call is isolated in `detectSystemLanguage()` behind `#ifdef __SWITCH__`, which keeps the module host-compilable and unit-testable.

### Time units

Durations are formatted from the `"h"` / `"mn"` source strings:

- **Sysmodule** (`i18n::formatDuration`, used by the notifications): English `"2h 30mn"`, French `"2h 30 min"`. The French minutes unit carries its leading space in the table (`" min"`).
- **Overlay** (usage values, history, setup-limits): the panels insert their own spacing around the unit strings (`"mn"` / `"min"`).

## Rules for contributors

- Every string that is rendered on screen (overlay) or sent in a notification (sysmodule) must go through `tr()` / `format()` with the English source text; do not hardcode user-facing literals in the panels or in `NotificationsController` / `UltraHandInterface`.
- To add a translatable string: call `tr("English text")` at the call site and add one row for that English text to the table of **every non-source language** of the component (`i18n/i18n_fr.cpp`, ...).
- To add a language: extend the `Language` enum, create the `i18n/i18n_<lang>.cpp` file with its table function, add its case to `tableFor()` in `i18n/i18n.cpp`, and add the language to the `detectSystemLanguage()` mapping.
- Keep the Doxygen comments and this document in sync when the mechanism changes.

## Tests

- Host: `make -C tests` runs `i18n_test.cpp`, which checks the English/French strings, the fallback to the source text for untranslated strings, the `%s` substitution, the duration formatting in both languages, and the host fallback of `detectSystemLanguage()`.
- The overlay tables cannot be host-tested (the overlay needs the Tesla framework), so keep them consistent by hand: every `tr("...")` / `format("...")` argument used in `overlay/src` must exist in `i18n/i18n_fr.cpp`, and every table row must be used.
