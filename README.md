# CECalc

`cecalc` is a Windows CE port of the Win2000 Scientific Calculator
(`Win2000SRC/private/windows/shell/accesory/calc/`), with the original
`ratpak` arbitrary-precision math engine vendored in.

Project layout:

- [`src/calc.cpp`](./src/calc.cpp) plus the `sci*.cpp` / `input.cpp` /
  `unifunc.cpp` / `wassert.cpp` files are the calc UI, ported from the
  Win2000 sources. They are compiled as C++ because the original code uses
  `try` / `catch (DWORD)` to handle ratpak errors.
- [`src/ratpak/`](./src/ratpak/) is the unmodified Win2000 ratpak math
  library (basex, conv, num, rat, exp, fact, itrans, logic, support, trans
  plus headers). Compiled as C, linked with the C++ side.
- [`src/ce_compat.h`](./src/ce_compat.h) and
  [`src/ce_compat.cpp`](./src/ce_compat.cpp) provide shims for desktop
  Win32 APIs that don't exist on CE: `HtmlHelp`, `WinHelp`, `ShellAbout`,
  `RegisterClassEx`, `CharNextA`, and the `Get/WriteProfile{Int,String}`
  family (redirected to `HKCU\Software\CECalc`).
- [`src/resources.rc`](./src/resources.rc) is the original calc resource
  script with the MSVC-only `winres.h`, TEXTINCLUDE, and DESIGNINFO blocks
  stripped so the CE `windres` accepts it.

## Build

```sh
make
```

Toolchain: `/opt/arm-mingw32ce/bin/arm-mingw32ce-*`, matching the other CE
projects in this workspace. Output is `cecalc.exe`.

## Runtime settings

`HKCU\Software\CECalc\SciCalc`:

- `layout` (0 = scientific, 1 = standard)
- `UseSep` (digit grouping toggle)

These map onto the original `WIN.INI [SciCalc]` keys; the registry layer is
in [`src/ce_compat.cpp`](./src/ce_compat.cpp).

## Caveats

- HTML Help and WinHelp are stubbed — pressing the help menu items does
  nothing on CE. The original calc shipped against `calc.chm` which has no
  CE equivalent.
- The clipboard paste handler still pastes `CF_TEXT` (ANSI) one byte at a
  time; the `CharNextA` shim advances by one byte, which is fine for the
  ASCII-range characters calc actually maps.
- The two main dialog templates (`IDD_SCIENTIFIC`, `IDD_STANDARD`) are sized
  for desktop Windows. They will load on CE but won't fit a Pocket PC
  screen without reflowing — consider that a follow-up.
