/* main.c — nd-level, ported to libxylem.
 *
 * Gives every entity a level, prints it in the status panel, and exposes
 * level()/level_up() to other modules. It is the slice's proof of all four of:
 *
 *   - a cross-module API: XY_IMPL here, XY_DECL in the consumer's TU, via
 *     include/level/level.h. The old SIC equivalent was a fn-pointer global
 *     inside `struct nd`, copied into every module at load by struct
 *     assignment; there is no such struct any more, and none is needed.
 *   - a module-owned corm table: level_hd comes from nd_open() and is a
 *     TAGGED handle (MODS.md §0.2), so it can never collide with an HD_*.
 *   - nd_printf instead of the variadic nd_writef.
 *   - a hook the engine fires (on_status) reaching a module.
 *
 * Original: tty-pt/nd-level @ 1015 B main.c, from the nd-basics superproject.
 *
 * This TU XY_IMPLs level, level_up, on_add and on_status, and so must NOT
 * include include/level/level.h or nd/hooks.h -- an XY_IMPL and an
 * XY_DECL of the same name in one TU is the XY equivalent of the old
 * SIC_DEF/SIC_DECL collision. A consumer includes the header and gets the
 * XY_DECLs; that is the whole point of keeping it in a separate file.
 */

#include <ttypt/xy-mod.h>

#include <nd/xy.h>

/* The module's own table, as a tagged module handle. Never an enum hd. */
static unsigned level_hd;

XY_MODULE_API void
xy_install(void)
{
	level_hd = nd_open("level", "u", "u", 0);
	WARN("nd-level: xy_install, level_hd = 0x%x (%s)\n", level_hd,
		nd_hd_is_mod(level_hd) ? "tagged" : "NOT TAGGED");
}

XY_IMPL(unsigned, level, unsigned, ref)
{
	unsigned lvl = 0;

	nd_get(level_hd, &lvl, &ref);
	return lvl;
}

XY_IMPL(int, level_up, unsigned, ref, unsigned, times)
{
	unsigned lvl;

	nd_get(level_hd, &lvl, &ref);
	lvl += times;
	if (times > 1)
		nd_printf(ref, "You level up (x%u)\n", times);
	else
		nd_printf(ref, "You level up!\n");
	nd_put(level_hd, &ref, &lvl);
	return 0;
}

XY_IMPL(int, on_add, unsigned, ref, unsigned, type, uint64_t, v)
{
	/* RENAMED from the original's `unsigned level = 3;`, which shadowed the
	 * level() function this very file defines. Legal C, actively confusing:
	 * the local is a value, the function is the module's API. */
	unsigned lvl = 3;

	(void)v;
	if (type != TYPE_ENTITY)
		return 1;

	nd_put(level_hd, &ref, &lvl);
	return 0;
}

XY_IMPL(int, on_status, unsigned, player_ref)
{
	nd_printf(player_ref, "Level\t%-8u\n", level(player_ref));
	return 0;
}
