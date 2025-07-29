#include <nd/nd.h>
#include <nd/type.h>

SIC_DECL(unsigned, level, unsigned, ref);
SIC_DEF(unsigned, level, unsigned, ref);

SIC_DECL(int, level_up, unsigned, ref, unsigned, times);
SIC_DEF(int, level_up, unsigned, ref, unsigned, times);

unsigned level_hd;

unsigned level(unsigned ref) {
	unsigned lvl;
	nd_get(level_hd, &lvl, &ref);
	return lvl;
}

int level_up(unsigned ref, unsigned times) {
	unsigned lvl;
	nd_get(level_hd, &lvl, &ref);
	lvl += times;
	if (times > 1)
		nd_writef(ref, "You level up (x%u)\n", times);
	else
		nd_writef(ref, "You level up!\n");
	nd_put(level_hd, &ref, &lvl);
	return 0;
}
int on_add(unsigned ref, unsigned type, uint64_t v __attribute__((unused))) {
	unsigned level = 3;

	if (type != TYPE_ENTITY)
		return 1;

	nd_put(level_hd, &ref, &level);
	return 0;
}

int
on_status(unsigned player_ref)
{
	nd_writef(player_ref, "Level\t%-8u\n", level(player_ref));
	return 0;
}

void mod_open(void) {
	level_hd = nd_open("level", "u", "u", 0);
}

void mod_install(void) {
	mod_open();
}
