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

#include <stdio.h>

#include "config.h"

#include "libavutil/cpu.h"
#include "libavutil/avstring.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if !HAVE_GETOPT
#include "compat/getopt.c"
#endif

static const struct {
    int flag;
    const char *name;
} cpu_flag_tab[] = {
    { 0 }
};

static void print_cpu_flags(int cpu_flags, const char *type)
{
    int i;

    printf("cpu_flags(%s) = 0x%08X\n", type, cpu_flags);
    printf("cpu_flags_str(%s) =", type);
    for (i = 0; cpu_flag_tab[i].flag; i++)
        if (cpu_flags & cpu_flag_tab[i].flag)
            printf(" %s", cpu_flag_tab[i].name);
    printf("\n");
}


int main(int argc, char **argv)
{
    int cpu_flags_raw = av_get_cpu_flags();
    int cpu_flags_eff;
    int cpu_count = av_cpu_count();
    char threads[5] = "auto";
    int i;

    for(i = 0; cpu_flag_tab[i].flag; i++) {
        unsigned tmp = 0;
        if (av_parse_cpu_caps(&tmp, cpu_flag_tab[i].name) < 0) {
            fprintf(stderr, "Table missing %s\n", cpu_flag_tab[i].name);
            return 4;
        }
    }

    if (cpu_flags_raw < 0)
        return 1;

    for (;;) {
        int c = getopt(argc, argv, "c:t:");
        if (c == -1)
            break;
        switch (c) {
        case 'c':
        {
            unsigned flags = av_get_cpu_flags();
            if (av_parse_cpu_caps(&flags, optarg) < 0)
                return 2;

            av_force_cpu_flags(flags);
            break;
        }
        case 't':
        {
            int len = av_strlcpy(threads, optarg, sizeof(threads));
            if (len >= sizeof(threads)) {
                fprintf(stderr, "Invalid thread count '%s'\n", optarg);
                return 2;
            }
        }
        }
    }

    cpu_flags_eff = av_get_cpu_flags();

    if (cpu_flags_eff < 0)
        return 3;

    print_cpu_flags(cpu_flags_raw, "raw");
    print_cpu_flags(cpu_flags_eff, "effective");
    printf("threads = %s (cpu_count = %d)\n", threads, cpu_count);

    return 0;
}
