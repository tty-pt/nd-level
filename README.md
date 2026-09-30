# axil-nd-level

`nd-level` for [axil-nd](../axil-nd), ported from SIC to libxylem.

Gives every entity a level, shows it in the status panel, and exports
`level()` / `level_up()` for other modules to call. It is the slice's most
complete exercise: a cross-module API, a module-owned corm table reached through
a tagged handle, `nd_printf`, and an event hook that puts text on a player's
socket.

It is also the only module of the ported set with a public header, and per
MODS.md §7 that header's shape is meant to be stable now that dependents build
on it.

## Install

```sh
make install
```

Installs two files:

```
lib/libnd-level.so
include/nd/level.h
```

There is deliberately no `lib/nd-level.so` symlink. `xy_load()` appends `.so`
itself and does not retry with a `lib` prefix, so the engine's `mods.load` names
this module `libnd-level` and `dlopen`s `libnd-level.so`. A soname symlink would
also have been silently dropped from the OpenBSD package: `tty-pt/ci` builds the
packing list from `find usr -type f`, which never lists a symlink, so the package
would have shipped the library under one name and asked the loader for another.

Also packaged for deb, apk, rpm, brew and openbsd from a `v*` tag.

## Build from source

```sh
make
```

Needs [libxylem](https://github.com/tty-pt/libxylem) (headers already in
`$(PREFIX)/include`) and the engine's game API, `<nd/xy.h>`, from either an
`axil-nd` checkout beside this repo or an installed `axil-nd`:

```sh
git clone https://github.com/tty-pt/nd-level && cd nd-level
git clone https://github.com/tty-pt/axil-nd ../axil-nd
make
```

`<nd/xy.h>` installs to `$(PREFIX)/include/nd/`, the same directory that already
carries `<ttypt/xy.h>`, so an **installed** engine needs no `-I` of its own here.
Against a checkout beside this repo it is `-I../axil-nd/include`; both paths are
on `CFLAGS` at once and a missing `-I` is ignored, so the same command works
either way.

## What it does

* `xy_install()` opens its own table: `nd_open("level", "u", "u", 0)`.
* `on_add(ref, type, v)` seeds every `TYPE_ENTITY` at level 3.
* `on_status(player_ref)` prints `Level\t<n>`, read back out of that table.
* `level(ref)` and `level_up(ref, times)` are the cross-module API.

## The cross-module API

`include/nd/level.h` holds the `XY_DECL`s:

```c
#include <ttypt/xy-mod.h>     // must come first: injects the xy context
#include <nd/xy.h>            // engine service hooks (nd_printf, ...)

#include <nd/level.h>   // this file
...
level_up(ref, 1);
```

`XY_DECL` generates a `static inline` in the consumer's TU that dispatches by
name, so the caller writes the bare name. `mk/portable.mk` puts `$(pwd)/include`
ahead of `$(PREFIX)/include`, so that one spelling resolves to this checkout in a
dev build and to the installed header otherwise.

The consumer does **not** have to load this module itself — the engine loads
everything in `mods.load` into one region and XY dispatches by name — but
`libnd-level` must be in the engine's `mods.load`, or `level()` returns 0 and
`level_up()` is a no-op rather than a crash.

Two further rules make this work, and both are load-bearing:

* Keep private headers in `include/ttypt/`, **not** `include/uapi/`. `uapi`
  belongs to the engine and is already on the include path, so a module with its
  own `uapi/` creates two `uapi` roots — the exact collision the module/engine
  header split exists to prevent.
* `src/libnd-level.c` must **not** include this header. It `XY_IMPL`s `level` and
  `level_up`; an `XY_IMPL` and an `XY_DECL` of the same name in one TU is the XY
  equivalent of the old `SIC_DEF`/`SIC_DECL` collision. The header exists
  precisely so implementation and consumer can be different TUs.

## Testing

There is no `test.sh` here. Behaviour is asserted by the engine's own suite,
which builds every module in its `mods.load`, boots, and greps stderr:

```sh
cd ../axil-nd
make && ./test.sh
```

The suite asserts the `Level` line this module prints actually reaches a socket,
and separately that `xy_install` logged a **tagged** handle
(`level_hd = 0x8…`) rather than a bare `0`.

## Notes from the port

* **The include changed spelling.** It was `"papi/nd-xy.h"`, a file that no
  longer exists in any checkout: the engine moved its module-facing tree from
  `papi/` to `nd/`, and `papi/` now holds only `nd.h`. The module could not
  compile until this was fixed.
* **The `on_add` local was renamed.** The original declared
  `unsigned level = 3;` — shadowing the `level()` function the same file
  defines. Legal C, and actively confusing: one name for a value and for the
  module's API. Renamed to `lvl`.
* The `HD_*` handles changed shape. `nd_open` used to throw away `corm_open`'s
  return and hand back `0`, so a module that saved the handle read and wrote corm
  table 0 — and could alias an `HD_*`. It now returns a **tagged** handle
  (`ND_HD_MOD | idx`), which cannot collide with an `enum hd` whatever corm hands
  out. `xy_install` logs the tag, and the test asserts it.
* `nd_writef` → `nd_printf`. The `va_list` version cannot be a hook at all, since
  a `va_list` does not survive the argument-struct copy across the bus;
  `nd_printf` formats module-side and writes with an explicit length.

## License

BSD 2-Clause, carried over from `tty-pt/nd-level`. See `LICENSE`.
