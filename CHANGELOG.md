## 1.0.0

- **nd-level is now an installable library rather than a build artifact of the
  engine.** It builds and installs two files: `lib/libnd-level.so` and the
  public header as `include/ttypt/nd-level.h`, following the same layout as
  `axil-tty` and `axil-auth`, and the same layout `nd-core` was converted to
  first. Previously `make` produced a `level.so` named by the engine's
  `mods.load` and installed nothing. There is no `lib/nd-level.so` symlink:
  `mods.load` names this module `libnd-level`, the installed filename, and
  `module_load_path()` only appends `.so`, so the load name must equal the
  installed name. A symlink would in any case have been dropped from the
  OpenBSD package, whose packing list is built from `find usr -type f`.

- **The cross-module API moved from `include/level/level.h` to
  `include/ttypt/nd-level.h`** and is now included as `<ttypt/nd-level.h>`.
  `mk/portable.mk` puts `$(pwd)/include` ahead of `$(PREFIX)/include`, so one
  spelling resolves to this checkout in a dev build and to the installed header
  otherwise. This is the only module of the ported set with a public header:
  `level()` and `level_up()` are called by other modules, and per MODS.md §7
  this header's shape is meant to be stable now that dependents build on it.

- **The link line is libxylem alone.** `LDLIBS := -lxylem`; the engine is not
  linked. A module is `dlopen`'d by an engine that already has XY resident, and
  XY_IMPL/XY_DECL resolve through the injected xy context, not through
  link-time symbols. `NEEDED` is `libxylem.so` and `libc.so.6`. `level_hd`
  stays module-local: it is a `static` table tag, not an exported symbol.

- **The game's service API is included as `<nd/xy.h>`**, from the engine's
  `$(PREFIX)/include/nd/` — the same include root as `<ttypt/xy.h>`, so this
  library needs no private `-I` for the game headers at all. It previously
  included `"papi/nd-xy.h"`, which no longer exists in any checkout: the engine
  moved the module-facing tree from `papi/` to `nd/` and `papi/` now holds only
  `nd.h`. That include could not compile, so this release also fixes the build.

- **Dropped the `nd-mod.mk` dependency.** This module resolves the game's
  headers itself, the way every other house library does, and `nd-mod.mk` is
  the SIC-era engine module build contract. It has now been deleted: this was
  the last kind of module that still included it.

- **`mods.load` names the installed filename.** A line with no in-tree module
  is passed to `xy_load()` verbatim so the dynamic linker resolves it. As
  before, the name carries no `.so` suffix — `xy_load()` appends it itself.
  Loading by soname rather than by sibling path means the consumer no longer
  has to be next to this checkout: the engine's `mods.load` must list
  `libnd-level` or `level()` returns 0 and `level_up()` is a no-op.
