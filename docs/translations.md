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

Each component has its own translation module (the two builds do not share sources — same convention as the duplicated IPC command enum):

| Component | Header | Implementation |
|-----------|--------|----------------|
| Sysmodule | `sysmodule/source/i18n.h` | `sysmodule/source/i18n.cpp` |
| Overlay   | `overlay/src/i18n.h`       | `overlay/src/i18n.cpp`       |

Both modules expose the same API in the namespace `alefbet::pctrl::i18n`:

- `enum class Language { English, French }`
- `enum class Key { ... }` — one enumerator per translatable string (the key list differs per component).
- `Language current()` — the active language.
- `void setLanguage(Language)` — override the language (used by the host tests).
- `void detectSystemLanguage()` — read the console language (SET service, target only).
- `const char* tr(Key)` — the string in the active language (empty string if the key is unknown).
- `std::string format(Key, const std::string&)` — like `tr`, but replaces the single `%s` placeholder of the template (e.g. the `TimeLeft` notification: `"%s left"` / `"Il reste %s"`).

The implementation is a single `constexpr std::array` table `{key, english, french}` plus a global `Language g_language`. The SET-service call is isolated in `detectSystemLanguage()` behind `#ifdef __SWITCH__`, which keeps the module host-compilable and unit-testable.

### Time units

Durations are formatted from the `UnitHours` / `UnitMinutes` keys:

- **Sysmodule** (`i18n::formatDuration`, used by the notifications): English `"2h 30mn"`, French `"2h 30 min"`. The French minutes unit carries its leading space in the table (`" min"`).
- **Overlay** (usage values, history, setup-limits): the panels insert their own spacing around the unit keys (`"mn"` / `"min"`).

## Rules for contributors

- Every string that is rendered on screen (overlay) or sent in a notification (sysmodule) must go through `tr()` / `format()`; do not hardcode user-facing literals in the panels or in `NotificationsController` / `UltraHandInterface`.
- To add a translatable string: add a `Key` enumerator and one row to the table of the component that shows it (English and French), then use `tr(Key::...)` at the call site.
- To add a language: extend the `Language` enum, add a column to the `Entry` struct and the table, extend `tr()` / `format()`, and add the language to the `detectSystemLanguage()` mapping.
- Keep the Doxygen comments and this document in sync when the mechanism changes.

## Tests

- Host: `make -C tests` runs `i18n_test.cpp`, which checks the English/French strings, the `%s` substitution, the duration formatting in both languages, and the host fallback of `detectSystemLanguage()`.
- The overlay table cannot be host-tested (the overlay needs the Tesla framework), so keep the overlay keys consistent by hand: every `Key::` used in `overlay/src` must exist in `overlay/src/i18n.h`, and vice versa.
