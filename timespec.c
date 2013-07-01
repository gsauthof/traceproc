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



#include "timespec.h"
#include "ret_check.h"

int timespec_add(struct timespec *res,
    const struct timespec *a)
{
  struct timespec old = *res;

  res->tv_sec += a->tv_sec;
  long sum = res->tv_nsec + a->tv_nsec;
  if (sum < 1000000000)
    res->tv_nsec = sum;
  else {
    ++res->tv_sec;
    res->tv_nsec = sum - 1000000000;
  }
  if (old.tv_sec > res->tv_sec)
    return -1;
  if (old.tv_sec == res->tv_sec && old.tv_nsec > res->tv_nsec)
    return -1;
  return 0;
}

int timespec_minus(struct timespec *res,
    const struct timespec *a)
{
  if (res->tv_sec < a->tv_sec)
    return -1;
  res->tv_sec -= a->tv_sec;
  if (res->tv_nsec >= a->tv_nsec)
    res->tv_nsec -= a->tv_nsec;
  else {
    if (!res->tv_sec)
      return -1;
    --res->tv_sec;
    long x = 1000000000 + (res->tv_nsec - a->tv_nsec);
    res->tv_nsec = x;
  }
  return 0;
}


int timespec_add_intv(struct timespec *res,
    const struct timespec *begin, const struct timespec *end)
{
  if (begin->tv_sec > end->tv_sec)
    return -1;
  if (begin->tv_sec == end->tv_sec
      && begin->tv_nsec > end->tv_nsec)
    return -1;
  struct timespec t = {0};
  // start end
  // 1:500 1:600
  // 1:500 1:500
  // 1:500 2:500
  // 1:500 2:400

  struct timespec a = *end;
  int ret = timespec_minus(&a, begin);
  IFTRUERET(ret, 0, ret);
  ret = timespec_add(res, &a);
  IFTRUERET(ret, 0, ret);
  return 0;
}

double timespec_cent(const struct timespec *part,
    const struct timespec *sum)
{
  return
    ((double)part->tv_sec * 1000000000.0 + (double)part->tv_nsec) /
    ((double)sum->tv_sec * 1000000000.0 + (double)sum->tv_nsec);
}

int timespec_pp_intv(const struct timespec *i,
    char *s, size_t n)
{
  char suffix[6] = {0};
  if (i->tv_nsec >= 1000000000)
    return -1;
  snprintf(suffix, 6, "%.3f", (double)i->tv_nsec/1000000000.0);
  snprintf(s, n, "%zu.%s", i->tv_sec, suffix+2);
  return 0;
}

size_t timespec_strftime(char *s, size_t max, const char *format,
    const struct timespec *t)
{
  if (!max)
    return 0;
  time_t secs = t->tv_sec;
  struct tm tm = {0};
  struct tm *ret_l = localtime_r(&secs, &tm);
  if (!ret_l)
    return 0;
  size_t ret_s = strftime(s, max, format, &tm);
  if (!ret_s)
    return 0;
  char out[max];
  *out = 0;
  const char *a = s;
  size_t written = 0;
  for (;;) {
    const char *x = strchr(a, '#');
    if (!x) {
      written += strlen(a);
      if (written >= max)
        return 0;
      strcat(out, a);
      strcpy(s, out);
      return written;
    }
    written += x-a;
    if (written >= max)
      return 0;
    strncat(out, a, x-a);
    switch (x[1]) {
      case '#':
        ++written;
        if (written >= max)
          return 0;
        strcat(out, "#");
        break;
      case '_':
        ++written;
        if (written >= max)
          return 0;
        strcat(out, " ");
        break;
      case 'S':
        {
          written += 3;
          if (written >= max)
            return 0;
          char sub[6] = {0};
          snprintf(sub, 6, "%.3f", (double)t->tv_nsec/1000000000.0);
          strcat(out, sub+2);
        }
        break;
      case 0:
        fprintf(stderr, "timespec_strftime: missing # code\n");
        return 0;
      default:
        fprintf(stderr, "timespec_strftime: unknown # code: %c\n",x[1]);
        return 0;
    }
    a = x+2;
  }
}


