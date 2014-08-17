#include <linux/linkage.h>
#include <linux/init.h>

ENTRY(meson_secondary_startup)
        bl	v7_invalidate_l1
        b       secondary_startup
ENDPROC(meson_secondary_startup)
