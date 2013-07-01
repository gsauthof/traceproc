/* {{{

    This file is part of libtraceproc - a library for tracing Pro*C/OCI calls

    Copyright (C) 2013 Georg Sauthoff <mail@georg.so>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

}}} */


#include "stats.h"

#include <stdlib.h>

#include "trace.h"
#include "timespec.h"
#include "ret_check.h"

int stats_init(Stats *stats, size_t type_size, size_t fn_size)
{
  stats->counts = calloc(type_size, sizeof(size_t));
  IFNULLRET(stats->counts, -1);
  stats->fn_counts = calloc(fn_size, sizeof(size_t));
  IFNULLRET(stats->fn_counts, -1);
  stats->times = calloc(type_size, sizeof(struct timespec));
  IFNULLRET(stats->times, -1);
  stats->fn_times = calloc(fn_size, sizeof(struct timespec));
  IFNULLRET(stats->fn_times, -1);
  return 0;
}

int stats_sum_up(Stats *stats, size_t type_size, size_t fn_size,
    const struct timespec *prog_time)
{
  if (prog_time)
    stats->prog_time = *prog_time;

  stats->count_sum = 0;
  stats->time_sum = (const struct timespec) {0};
  for (size_t i = 0; i<type_size; ++i) {
    stats->count_sum += stats->counts[i];
    int ret = timespec_add(&stats->time_sum, stats->times + i);
    IFTRUEEXIT(ret, 0, 10);
  }

  stats->fn_count_sum = 0;
  stats->fn_time_sum = (const struct timespec) {0};
  for (size_t i = 0; i<fn_size; ++i) {
    stats->fn_count_sum += stats->fn_counts[i];
    int ret = timespec_add(&stats->fn_time_sum, stats->fn_times + i);
    IFTRUEEXIT(ret, 0, 10);
  }

  return 0;
}

int stats_pp_fns(const Stats *stats, char const * const * names,
    unsigned n)
{
  tprintf("\n%20s | %10s | %14s | %4s\n",
      "FUNCTION", "COUNT", "TIME (s)", "%");
  tprintf("---------------------+------------+----------------+-----\n");

  struct timespec other = stats->prog_time;
  for (unsigned i = 0; i<n; ++i) {
    if (!stats->fn_counts[i])
      continue;
    char t[15] = {0};
    timespec_pp_intv(&stats->fn_times[i], t, 15);

    tprintf("%20s | %10zu | %14s | %.2f\n",
        names[i],
        stats->fn_counts[i],
        t,
        timespec_cent(&stats->fn_times[i], &stats->prog_time)
        );
    timespec_minus(&other, &stats->fn_times[i]);
  }
  char t[15] = {0};
  timespec_pp_intv(&other, t, 15);
  tprintf("%20s | %10s | %14s | %.2f\n",
      "non-sql", "x", t, timespec_cent(&other, &stats->prog_time));
  tprintf("=====================+============+================+=====\n");
  t[0] = 0;
  //timespec_pp_intv(&stats->fn_time_sum, t, 15);
  timespec_pp_intv(&stats->prog_time, t, 15);
  tprintf("%20s | %10zu | %14s | %4s\n",
      "SUM",stats->fn_count_sum, t, "1.00");
  return 0;
}
