/*
 * SMP entry code for Amlogic Meson SoCs
 *
 * Copyright (C) 2014 Beniamino Galvani <b.galvani@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/linkage.h>
#include <linux/init.h>

.global meson_secondary_trampoline_jump

ENTRY(meson_secondary_trampoline)
	ldr	pc, 1f
meson_secondary_trampoline_jump:
1:	.space	4
ENDPROC(meson_secondary_trampoline)

ENTRY(meson_secondary_startup)
        bl	v7_invalidate_l1
        b       secondary_startup
ENDPROC(meson_secondary_startup)
