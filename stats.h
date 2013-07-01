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


#ifndef STATS_H
#define STATS_H

#include <stddef.h>
#include <time.h>

struct Stats {
  size_t *counts;
  size_t count_sum;
  size_t *fn_counts;
  size_t fn_count_sum;

  struct timespec *times;
  struct timespec time_sum;
  struct timespec *fn_times;
  struct timespec fn_time_sum;

  struct timespec prog_start;
  struct timespec prog_end;
  struct timespec prog_time;
};
typedef struct Stats Stats;


#define STATS_BEGIN(A) \
  struct timespec time_begin = {0}, time_end = {0}; \
  do { \
  if (A) { \
    int ret = clock_gettime(WRAP_CLOCK_ID, &time_begin); \
    IFERRNOEXIT(ret, 0, 10); \
  } \
  } while (0)

#define STATS_END(A, B, C) \
  do { \
  if (A) { \
    int ret = clock_gettime(WRAP_CLOCK_ID, &time_end); \
    IFERRNOEXIT(ret, 0, 10); \
    ++stats.fn_counts[B]; \
    ret = timespec_add_intv(stats.fn_times + B,\
        &time_begin, &time_end); \
    IFTRUEEXIT(ret, 0, 10); \
    if (C) { \
      ++stats.counts[C]; \
      ret = timespec_add_intv(stats.times + C, &time_begin, &time_end); \
      IFTRUEEXIT(ret, 0, 10); \
    } \
  } \
  } while (0)


__attribute__((visibility("hidden")))
int stats_init(Stats *stats, size_t type_size, size_t fn_size);

__attribute__((visibility("hidden")))
int stats_sum_up(Stats *stats, size_t type_size, size_t fn_size,
    const struct timespec *prog_time);

__attribute__((visibility("hidden")))
int stats_pp_fns(const Stats *stats, char const * const * names,
    unsigned n);

#endif
