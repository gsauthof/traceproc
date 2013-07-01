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


#ifndef UNITTEST_BASIC_H
#define UNITTEST_BASIC_H

int select_date(const char *str, char *output, size_t n);
int select_date_charz(const char *str, char *output, size_t n);
int select_date_varchar(const char *str, char *output, size_t n, unsigned short *len);
int char_type(char c, char *d);
int create_table();
int drop_syn();
int create_syn();
int create_pub_syn();

#endif
