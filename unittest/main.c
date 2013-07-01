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


#include <stdbool.h>
#include <stdlib.h>

#include <check.h>


#include "test_timespec.h"
#include "test_parsesql.h"
#include "test_trace.h"



// XXX coverage testing
int main(int argc, char **argv)
{
  bool sel[64] = {0};
  if (argc > 1) {
    unsigned a = atoi(argv[1]);
    if (a<64)
      sel[a] = true;
  } else
    for (unsigned x = 0; x<64; ++x)
      sel[x] = true;

  Suite *s = suite_create("libtraceproc");

//  TCase *tc_basic = tcase_create("Basic");
//  tcase_add_test(tc_basic, ref_example);
//  suite_add_tcase(s, tc_basic);

  unsigned i = 0;
  TCase *tc_timespec = timespec_tc_create();
  if (sel[i] && tc_timespec)
    suite_add_tcase(s, tc_timespec);
  ++i;
  TCase *tc_parsesql = parsesql_tc_create();
  if (sel[i] && tc_parsesql)
    suite_add_tcase(s, tc_parsesql);
  ++i;
  TCase *tc_trace = trace_tc_create();
  if (sel[i] && tc_trace)
    suite_add_tcase(s, tc_trace);

  SRunner *sr = srunner_create(s);
  //srunner_run_all(sr, CK_NORMAL);
  // default CK_NORMAL, else from environment variable CK_VERBOSITY
  srunner_run_all(sr, CK_ENV);
  int failed = srunner_ntests_failed(sr);

  srunner_free(sr);

  return failed != 0;
}
