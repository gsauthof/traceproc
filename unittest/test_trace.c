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


#include "test_trace.h"

#include <trace.h>

#include <stdlib.h>
#include <string.h>


#include <unistd.h>
#include <pthread.h>
#include <errno.h>



static int pipefd[2] = {0};

static pthread_t slave_id = 0;

static char buffer[1024] = {0};

static void *slave_fn(void *arg)
{
  memset(buffer, 0, 1024);
  int fd = *(int*)arg;
  size_t left = 1024;
  char *p = buffer;
  for (;;) {
    ssize_t n = read(fd, p, left);
    if (n<0)
      return (void*) -1;
    if (!n)
      break;
    left -= n;
    p += n;
  }
  return (void*) 0;
}

START_TEST(tprintf_01)
{
  //ck_abort_msg("da fuck");
  trace_set_ftime("#_##");

  FILE *file = fdopen(pipefd[1], "w");
  ck_assert_msg(file != 0, "fdopen() failed: %s", strerror(errno));
  trace_set_file(file);

  tprintf("%d %s\n", 123, "foo bar");

  int ret_fflush = fflush(file);

  ck_assert_msg(!ret_fflush, "fflush failed: %s", strerror(errno));
  int ret_close = close(pipefd[1]);
  ck_assert_msg(!ret_close, strerror(errno));

  void *ret_val = 0;
  int ret_join = pthread_join(slave_id, &ret_val);
  ck_assert_int_eq(ret_join, 0);
  ck_assert_int_ne(ret_val, PTHREAD_CANCELED);
  ck_assert_int_eq(ret_val, 0);
  ck_assert_str_eq(buffer, " #123 foo bar\n");
}
END_TEST



// checked fixture - runs for each test! (unchecked only runs for the testcase)
static void setup()
{
  // ck_/fail fns also work in those functions!
  //ck_abort_msg("setup fail");
  int ret_pipe = pipe(pipefd);
  ck_assert_int_eq(ret_pipe, 0);
  int ret_cre = pthread_create(&slave_id, 0, slave_fn, pipefd);
  ck_assert_int_eq(ret_cre, 0);
}

static void teardown()
{
}

TCase *trace_tc_create()
{
  TCase *tc = tcase_create("trace");
  // alternative: tcase_add_unchecked_fixture ...
  tcase_add_checked_fixture(tc, setup, teardown);
  tcase_add_test(tc, tprintf_01);
  return tc;
}

