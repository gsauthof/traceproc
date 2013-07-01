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


#ifndef PARSESQL_H
#define PARSESQL_H


#include <stdbool.h>

struct SQL_Token {
  bool host_var;
  const char *begin;
  const char *end;
};
typedef struct SQL_Token SQL_Token;


int sql_tokenize(const char *s,
    SQL_Token **tokens, size_t *size);

int sql_pseudo_tokenize(size_t n,
    SQL_Token **tokens, size_t *size);

int sql_quote_str(const char *input, size_t input_size, char *output,
    size_t output_size);

#endif
