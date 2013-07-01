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


#ifndef ORA_UTIL_H
#define ORA_UTIL_H


struct Ora_Connect {
  char username[32];
  char password[32];
  char dbspec[32];
};
typedef struct Ora_Connect Ora_Connect;

int ora_init_vars2(char *username, char *password, char *dbspec,
    size_t size);

/* Part of proc_connect() such that Pro*C options
 * like max_row_insert have an effect (max_row_insert flags
 * the cud of CONNECT).
 */
int ora_init_vars(Ora_Connect *pc);

/** Reset SIGABRT, SIGSEGV etc. to default values.
 *
 * Call it after CONNECT because Oracle libcltsh.so installs own
 * handlers - which may introduce non-deterministic behaviour
 * in case of an abort/segfault.
 *
 * cf. http://stackoverflow.com/q/17124881/427158
 *
 */
int ora_reset_signals();

int ora_print_signals(const char *label);

#endif
