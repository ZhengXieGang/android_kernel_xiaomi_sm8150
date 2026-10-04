// SPDX-License-Identifier: GPL-2.0
/*
 * Minimal schedtune cgroup controller (control interface only) for the
 * crDroid Android 16 port on this 5.4 tree.
 *
 * Why this file exists: the ROM's userspace hard-requires a "schedtune"
 * cgroup v1 controller.
 *   - init.rc creates /dev/stune/{foreground,background,top-app,rt} and
 *     writes /dev/stune/nnapi-hal/schedtune.{boost,prefer_idle};
 *   - libprocessgroup mounts every controller it knows about during
 *     SetupCgroups and treats a failed mount as fatal: with schedtune
 *     missing, the command fails, every subsequent service start dies in
 *     createProcessGroup(), and init reboots with
 *     "reboot,bootloader,bootstrap-apexd-failed" ~10 s into the boot
 *     (before ueventd or apexd-bootstrap can ever run).
 *
 * The stock 4.14 kernel provides this through CONFIG_SCHED_TUNE (the CAF
 * schedtune/EAS integration).  This tree does not carry that driver, so
 * this port provides the control interface: the controller, its per-group
 * attributes and the value bookkeeping.  The scheduler-side boost (EAS
 * task placement / freq selection) is intentionally not wired in -- no
 * code in this tree consumes the values yet -- so writes are recorded and
 * read back faithfully, but do not alter scheduling.  That is exactly what
 * the boot path needs; the boost effect can be added later.
 */
#include <linux/cgroup.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/slab.h>

struct schedtune {
	struct cgroup_subsys_state css;

	/* Boost value for tasks on that SchedTune CGroup (0..100). */
	int boost;

	/* Hint to bias scheduling of tasks on that CGroup toward idle CPUs. */
	int prefer_idle;

	/* Accepted and remembered for interface compatibility. */
	int colocate;
};

static inline struct schedtune *css_st(struct cgroup_subsys_state *css)
{
	return css ? container_of(css, struct schedtune, css) : NULL;
}

/* Root control group: system-wide defaults, inherited by new groups. */
static struct schedtune root_schedtune;

static struct cgroup_subsys_state *
schedtune_css_alloc(struct cgroup_subsys_state *parent_css)
{
	struct schedtune *parent = css_st(parent_css);
	struct schedtune *st;

	if (!parent)
		return &root_schedtune.css;

	st = kzalloc(sizeof(*st), GFP_KERNEL);
	if (!st)
		return ERR_PTR(-ENOMEM);

	st->boost = parent->boost;
	st->prefer_idle = parent->prefer_idle;
	st->colocate = parent->colocate;
	return &st->css;
}

static void schedtune_css_free(struct cgroup_subsys_state *css)
{
	struct schedtune *st = css_st(css);

	if (st != &root_schedtune)
		kfree(st);
}

#define ST_ATTR(name, field)						\
	static u64 name##_read(struct cgroup_subsys_state *css,		\
			       struct cftype *cft)			\
	{								\
		return (u64)css_st(css)->field;				\
	}								\
	static int name##_write(struct cgroup_subsys_state *css,	\
				struct cftype *cft, u64 val)		\
	{								\
		css_st(css)->field = (int)val;				\
		return 0;						\
	}

static u64 st_boost_read(struct cgroup_subsys_state *css,
			 struct cftype *cft)
{
	return (u64)css_st(css)->boost;
}

static int st_boost_write(struct cgroup_subsys_state *css,
			  struct cftype *cft, u64 val)
{
	if (val > 100)		/* same clamp as the CAF implementation */
		val = 100;
	css_st(css)->boost = (int)val;
	return 0;
}

ST_ATTR(st_prefer_idle, prefer_idle)
ST_ATTR(st_colocate, colocate)

static struct cftype files[] = {
	{
		.name = "schedtune.boost",
		.read_u64 = st_boost_read,
		.write_u64 = st_boost_write,
	},
	{
		.name = "schedtune.prefer_idle",
		.read_u64 = st_prefer_idle_read,
		.write_u64 = st_prefer_idle_write,
	},
	{
		.name = "schedtune.colocate",
		.read_u64 = st_colocate_read,
		.write_u64 = st_colocate_write,
	},
	{ }	/* terminate */
};

struct cgroup_subsys schedtune_cgrp_subsys = {
	.css_alloc	= schedtune_css_alloc,
	.css_free	= schedtune_css_free,
	.legacy_cftypes	= files,
	.early_init	= false,
};
