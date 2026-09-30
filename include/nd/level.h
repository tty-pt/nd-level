/* level.h — nd-level's cross-module API.
 *
 * Include this from a module TU that wants to read or change levels, and NOT
 * from nd-level's own main.c: an XY_DECL and an XY_IMPL of the same name in
 * one TU collides, which is the direct replacement for the old
 * `SIC_DECL` + `SIC_DEF` pairing in a single file.
 *
 * Usage:
 *
 *     #include <ttypt/xy-mod.h>     // must come first: injects the xy context
 *     #include <nd/xy.h>            // engine service hooks (nd_printf, ...)
 *     #include <nd/level.h>   // this file
 *
 *     XY_IMPL(int, on_enter, unsigned, player_ref, unsigned, loc_ref) {
 *             level_up(player_ref, 1);
 *             nd_printf(player_ref, "you are level %u\n", level(player_ref));
 *             return 0;
 *     }
 *
 * The consumer does not need to load nd-level itself -- the engine loads every
 * module in mods.load into one region and XY dispatches by name -- but the
 * engine's mods.load must list level, or level() returns 0 and level_up() is a
 * no-op rather than a crash.
 *
 * NOTE: this is a MODULE-OWNED header, not an engine one. It lives in
 * include/level/ rather than include/uapi/ on purpose: `uapi` belongs to the
 * engine and is already on the include path via ND_INC, so a module putting
 * its own uapi/ there would produce two uapi roots and the nd-xy-types.h
 * collision that MODS.md §3 exists to prevent.
 */

#ifndef AXIL_ND_LEVEL_H
#define AXIL_ND_LEVEL_H

#include <ttypt/xy.h>

XY_DECL(unsigned, level, unsigned, ref);
XY_DECL(int, level_up, unsigned, ref, unsigned, times);

#endif
