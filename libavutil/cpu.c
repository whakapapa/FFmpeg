/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#include "attributes.h"
#include "cpu.h"
#include "cpu_internal.h"
#include "config.h"
#include "opt.h"
#include "common.h"

#if HAVE_SCHED_GETAFFINITY
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <sched.h>
#endif
#if HAVE_GETPROCESSAFFINITYMASK || HAVE_WINRT
#include <windows.h>
#endif
#if HAVE_SYSCTL
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

static atomic_int cpu_flags = ATOMIC_VAR_INIT(-1);

static int get_cpu_flags(void)
{
    return 0;
}


    atomic_store_explicit(&cpu_flags, arg, memory_order_relaxed);
}

int av_get_cpu_flags(void)
{
    int flags = atomic_load_explicit(&cpu_flags, memory_order_relaxed);
    if (flags == -1) {
        flags = get_cpu_flags();
        atomic_store_explicit(&cpu_flags, flags, memory_order_relaxed);
    }
    return flags;
}

void av_set_cpu_flags_mask(int mask)
{
    atomic_store_explicit(&cpu_flags, get_cpu_flags() & mask,
                          memory_order_relaxed);
}

int av_parse_cpu_flags(const char *s)
{
#define CPUFLAG_3DNOWEXT (AV_CPU_FLAG_3DNOWEXT | CPUFLAG_3DNOW)
#define CPUFLAG_SSE      (AV_CPU_FLAG_SSE      | CPUFLAG_MMXEXT)
#define CPUFLAG_SSE2     (AV_CPU_FLAG_SSE2     | CPUFLAG_SSE)
#define CPUFLAG_SSE2SLOW (AV_CPU_FLAG_SSE2SLOW | CPUFLAG_SSE2)
#define CPUFLAG_SSE3     (AV_CPU_FLAG_SSE3     | CPUFLAG_SSE2)
#define CPUFLAG_SSE3SLOW (AV_CPU_FLAG_SSE3SLOW | CPUFLAG_SSE3)
#define CPUFLAG_SSSE3    (AV_CPU_FLAG_SSSE3    | CPUFLAG_SSE3)
#define CPUFLAG_SSE4     (AV_CPU_FLAG_SSE4     | CPUFLAG_SSSE3)
#define CPUFLAG_SSE42    (AV_CPU_FLAG_SSE42    | CPUFLAG_SSE4)
#define CPUFLAG_AVX      (AV_CPU_FLAG_AVX      | CPUFLAG_SSE42)
#define CPUFLAG_AVXSLOW  (AV_CPU_FLAG_AVXSLOW  | CPUFLAG_AVX)
#define CPUFLAG_XOP      (AV_CPU_FLAG_XOP      | CPUFLAG_AVX)
#define CPUFLAG_FMA3     (AV_CPU_FLAG_FMA3     | CPUFLAG_AVX)
#define CPUFLAG_FMA4     (AV_CPU_FLAG_FMA4     | CPUFLAG_AVX)
#define CPUFLAG_AVX2     (AV_CPU_FLAG_AVX2     | CPUFLAG_AVX)
#define CPUFLAG_BMI2     (AV_CPU_FLAG_BMI2     | AV_CPU_FLAG_BMI1)
#define CPUFLAG_AESNI    (AV_CPU_FLAG_AESNI    | CPUFLAG_SSE42)
#define CPUFLAG_AVX512   (AV_CPU_FLAG_AVX512   | CPUFLAG_AVX2)
    static const AVOption cpuflags_opts[] = {
        { "flags"   , NULL, 0, AV_OPT_TYPE_FLAGS, { .i64 = 0 }, INT64_MIN, INT64_MAX, .unit = "flags" },
        { NULL },
    };
    static const AVClass class = {
        .class_name = "cpuflags",
        .item_name  = av_default_item_name,
        .option     = cpuflags_opts,
        .version    = LIBAVUTIL_VERSION_INT,
    };

    int flags = 0, ret;
    const AVClass *pclass = &class;

    if ((ret = av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, &flags)) < 0)
        return ret;

    return flags & INT_MAX;
}

int av_parse_cpu_caps(unsigned *flags, const char *s)
{
        static const AVOption cpuflags_opts[] = {
        { "flags"   , NULL, 0, AV_OPT_TYPE_FLAGS, { .i64 = 0 }, INT64_MIN, INT64_MAX, .unit = "flags" },

#define CPU_FLAG_P4 CPU_FLAG_P3| AV_CPU_FLAG_SSE2
        { "pentium2", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_P2          },    .unit = "flags" },
        { "pentium3", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_P3          },    .unit = "flags" },
        { "pentium4", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = CPU_FLAG_P4          },    .unit = "flags" },

#define CPU_FLAG_ATHLONXP CPU_FLAG_ATHLON | AV_CPU_FLAG_SSE
#endif
        { NULL },
    };
    static const AVClass class = {
        .class_name = "cpuflags",
        .item_name  = av_default_item_name,
        .option     = cpuflags_opts,
        .version    = LIBAVUTIL_VERSION_INT,
    };
    const AVClass *pclass = &class;

    return av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, flags);
}

int av_cpu_count(void)
{
    static volatile int printed;

    int nb_cpus = 1;
#if HAVE_WINRT
    SYSTEM_INFO sysinfo;
#endif
#if HAVE_SCHED_GETAFFINITY && defined(CPU_COUNT)
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    if (!sched_getaffinity(0, sizeof(cpuset), &cpuset))
        nb_cpus = CPU_COUNT(&cpuset);
#elif HAVE_GETPROCESSAFFINITYMASK
    DWORD_PTR proc_aff, sys_aff;
    if (GetProcessAffinityMask(GetCurrentProcess(), &proc_aff, &sys_aff))
        nb_cpus = av_popcount64(proc_aff);
#elif HAVE_SYSCTL && defined(HW_NCPU)
    int mib[2] = { CTL_HW, HW_NCPU };
    size_t len = sizeof(nb_cpus);

    if (sysctl(mib, 2, &nb_cpus, &len, NULL, 0) == -1)
        nb_cpus = 0;
#elif HAVE_SYSCONF && defined(_SC_NPROC_ONLN)
    nb_cpus = sysconf(_SC_NPROC_ONLN);
#elif HAVE_SYSCONF && defined(_SC_NPROCESSORS_ONLN)
    nb_cpus = sysconf(_SC_NPROCESSORS_ONLN);
#elif HAVE_WINRT
    GetNativeSystemInfo(&sysinfo);
    nb_cpus = sysinfo.dwNumberOfProcessors;
#endif

    if (!printed) {
        av_log(NULL, AV_LOG_DEBUG, "detected %d logical cores\n", nb_cpus);
        printed = 1;
    }

    return nb_cpus;
}

size_t av_cpu_max_align(void)
{

    return 8;
}
