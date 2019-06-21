// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * MIPS64 and compat userspace implementations of gettimeofday()
 * and similar.
 *
 * Copyright (C) 2015 Imagination Technologies
 * Copyright (C) 2018 ARM Limited
 *
 */
#include <linux/time.h>
#include <linux/types.h>

#if _MIPS_SIM != _MIPS_SIM_ABI64
int __vdso_clock_gettime(clockid_t clock,
			 struct old_timespec32 *ts)
{
	return __cvdso_clock_gettime32(clock, ts);
}

int __vdso_gettimeofday(struct __kernel_old_timeval *tv,
			struct timezone *tz)
{
	return __cvdso_gettimeofday(tv, tz);
}

#else

int __vdso_clock_gettime(clockid_t clock,
			 struct __kernel_timespec *ts)
{
	return __cvdso_clock_gettime(clock, ts);
}

int __vdso_gettimeofday(struct __kernel_old_timeval *tv,
			struct timezone *tz)
{
	return __cvdso_gettimeofday(tv, tz);
}

#endif
