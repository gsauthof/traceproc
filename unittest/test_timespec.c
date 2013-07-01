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


#include "test_timespec.h"
#include <timespec.h>

#include <math.h>

START_TEST(add_01)
{
  struct timespec a = {
    .tv_sec = 0,
    .tv_nsec = 0
  };
  struct timespec b = {
    .tv_sec = 13,
    .tv_nsec = 9
  };
  int ret = timespec_add(&a, &b);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 13);
  ck_assert_int_eq(a.tv_nsec, 9);
}
END_TEST

START_TEST(add_02)
{
  struct timespec a = {
    .tv_sec = 3,
    .tv_nsec = 700000000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 300000000
  };
  timespec_add(&a, &b);
  ck_assert_int_eq(a.tv_sec, 6);
  ck_assert_int_eq(a.tv_nsec, 0);
}
END_TEST

START_TEST(add_03)
{
  struct timespec a = {
    .tv_sec = 3,
    .tv_nsec = 400000000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 600010000
  };
  timespec_add(&a, &b);
  ck_assert_int_eq(a.tv_sec, 6);
  ck_assert_int_eq(a.tv_nsec, 10000);
}
END_TEST

#include <limits.h>

START_TEST(add_04)
{
  struct timespec a = {
    .tv_sec = INT_MAX,
    .tv_nsec = 400000000
  };
  struct timespec b = {
    .tv_sec = INT_MAX,
    .tv_nsec = 600010000
  };
  int ret1 = timespec_add(&a, &b);
  struct timespec c = {
    .tv_sec = LONG_MAX,
    .tv_nsec = 400000000
  };
  struct timespec d = {
    .tv_sec = LONG_MAX,
    .tv_nsec = 600010000
  };
  int ret2 = timespec_add(&c, &d);
  ck_assert(ret1 || ret2);
}
END_TEST

START_TEST(minus_01)
{
  struct timespec a = {
    .tv_sec = 3,
    .tv_nsec = 400000000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 600010000
  };
  int ret = timespec_minus(&a, &b);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 0);
  ck_assert_int_eq(a.tv_nsec, 799990000);
}
END_TEST

START_TEST(minus_02)
{
  struct timespec a = {
    .tv_sec = 3,
    .tv_nsec = 600010000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 400000000
  };
  timespec_minus(&a, &b);
  //ck_assert_int_eq(a.tv_sec, 0);
  ck_assert_int_eq(a.tv_sec, 1);
  ck_assert_int_eq(a.tv_nsec, 200010000);
}
END_TEST

START_TEST(minus_03)
{
  struct timespec a = {
    .tv_sec = 5,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  timespec_minus(&a, &b);
  ck_assert_int_eq(a.tv_sec, 3);
  ck_assert_int_eq(a.tv_nsec, 0);
}
END_TEST

START_TEST(minus_04)
{
  struct timespec a = {
    .tv_sec = 2,
    .tv_nsec = 400000000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  int ret = timespec_minus(&a, &b);
  ck_assert_int_ne(ret, 0);
}
END_TEST

START_TEST(minus_05)
{
  struct timespec a = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 3,
    .tv_nsec = 500000000
  };
  int ret = timespec_minus(&a, &b);
  ck_assert_int_ne(ret, 0);
}
END_TEST

START_TEST(add_intv_01)
{
  struct timespec a = {
    .tv_sec = 1,
    .tv_nsec = 300000000
  };
  struct timespec b = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  struct timespec c = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  int ret = timespec_add_intv(&a, &b, &c);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 2);
  ck_assert_int_eq(a.tv_nsec, 300000000);
}
END_TEST

START_TEST(add_intv_02)
{
  struct timespec a = {
    .tv_sec = 1,
    .tv_nsec = 300000000
  };
  struct timespec b = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  struct timespec c = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  int ret = timespec_add_intv(&a, &c, &b);
  ck_assert_int_ne(ret, 0);
}
END_TEST

START_TEST(add_intv_03)
{
  struct timespec a = {
    .tv_sec = 1,
    .tv_nsec = 800000000
  };
  struct timespec b = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  struct timespec c = {
    .tv_sec = 2,
    .tv_nsec = 800000000
  };
  int ret = timespec_add_intv(&a, &b, &c);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 3);
  ck_assert_int_eq(a.tv_nsec, 100000000);
}
END_TEST

START_TEST(add_intv_04)
{
  struct timespec a = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  struct timespec c = {
    .tv_sec = 2,
    .tv_nsec = 0
  };
  int ret = timespec_add_intv(&a, &b, &c);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 2);
  ck_assert_int_eq(a.tv_nsec, 0);
}
END_TEST

START_TEST(add_intv_05)
{
  struct timespec a = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  struct timespec c = {
    .tv_sec = 1,
    .tv_nsec = 500000000
  };
  int ret = timespec_add_intv(&a, &b, &c);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 2);
  ck_assert_int_eq(a.tv_nsec, 500000000);
}
END_TEST

START_TEST(add_intv_06)
{
  struct timespec a = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 1,
    .tv_nsec = 800000000
  };
  struct timespec c = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  int ret = timespec_add_intv(&a, &b, &c);
  ck_assert_int_eq(ret, 0);
  ck_assert_int_eq(a.tv_sec, 3);
  ck_assert_int_eq(a.tv_nsec, 200000000);
}
END_TEST

START_TEST(cent_01)
{
  struct timespec a = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 5,
    .tv_nsec = 0
  };
  double c = timespec_cent(&a, &b);
  ck_assert(fabs(c-0.5)<0.000001);
}
END_TEST

START_TEST(cent_02)
{
  struct timespec a = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  struct timespec b = {
    .tv_sec = 2,
    .tv_nsec = 500000000
  };
  double c = timespec_cent(&a, &b);
  ck_assert(fabs(c-1.0)<0.000001);
}
END_TEST

START_TEST(cent_03)
{
  struct timespec a = {
    .tv_sec = 66,
    .tv_nsec = 666666666
  };
  struct timespec b = {
    .tv_sec = 3*66+1,
    .tv_nsec = 99999998
  };
  double c = timespec_cent(&a, &b);
  ck_assert(fabs(c-0.3333)<0.01);
}
END_TEST

START_TEST(pp_intv_01)
{
  char out[10] = {0};
  struct timespec a = {
    .tv_sec = 66,
    .tv_nsec = 666666666 
  };
  int ret = timespec_pp_intv(&a, out, 10);
  ck_assert_int_eq(ret, 0);
  ck_assert(!strcmp(out, "66.666") || !strcmp(out, "66.667"));
}
END_TEST

START_TEST(pp_intv_02)
{
  char out[6] = {0};
  struct timespec a = {
    .tv_sec = 66,
    .tv_nsec = 666666666 
  };
  int ret = timespec_pp_intv(&a, out, 6);
  ck_assert_int_eq(ret, 0);
  ck_assert(!out[5]);
  ck_assert_str_eq(out, "66.66");
}
END_TEST

START_TEST(strft_01)
{
  char out[30] = {0};
  struct timespec a = {
    .tv_sec = 666666666,
    .tv_nsec = 4096 
  };
  size_t ret = timespec_strftime(out, 30, "%F_%H:%M:%S.#S:#_", &a);
  ck_assert_int_eq(ret, 25);
  ck_assert_str_eq(out, "1991-02-16_02:11:06.000: ");
}
END_TEST

START_TEST(strft_02)
{
  char out[30] = {0};
  struct timespec a = {
    .tv_sec = 666666666,
    .tv_nsec = 4096 
  };
  size_t ret = timespec_strftime(out, 25, "%F_%H:%M:%S.#S:#_", &a);
  ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(strft_03)
{
  char out[30] = {0};
  struct timespec a = {
    .tv_sec = 666666666,
    .tv_nsec = 4096 
  };
  size_t ret = timespec_strftime(out, 26, "%F_%H:%M:%S.#S:#_", &a);
  ck_assert_int_eq(ret, 25);
}
END_TEST

START_TEST(strft_04)
{
  char out[30] = {0};
  struct timespec a = {
    .tv_sec = 666666666,
    .tv_nsec = 4096
  };
  size_t ret = timespec_strftime(out, 26, "#_##", &a);
  ck_assert_int_eq(ret, 2);
  ck_assert_str_eq(out, " #");
}
END_TEST

TCase *timespec_tc_create()
{
  TCase *tc = tcase_create("timespec");
  tcase_add_test(tc, add_01);
  tcase_add_test(tc, add_02);
  tcase_add_test(tc, add_03);
  tcase_add_test(tc, add_04);
  tcase_add_test(tc, minus_01);
  tcase_add_test(tc, minus_02);
  tcase_add_test(tc, minus_03);
  tcase_add_test(tc, minus_04);
  tcase_add_test(tc, minus_05);
  tcase_add_test(tc, add_intv_01);
  tcase_add_test(tc, add_intv_02);
  tcase_add_test(tc, add_intv_03);
  tcase_add_test(tc, add_intv_04);
  tcase_add_test(tc, add_intv_05);
  tcase_add_test(tc, add_intv_06);
  tcase_add_test(tc, cent_01);
  tcase_add_test(tc, cent_02);
  tcase_add_test(tc, cent_03);
  tcase_add_test(tc, pp_intv_01);
  tcase_add_test(tc, pp_intv_02);
  tcase_add_test(tc, strft_01);
  tcase_add_test(tc, strft_02);
  tcase_add_test(tc, strft_03);
  tcase_add_test(tc, strft_04);
  return tc;
}


