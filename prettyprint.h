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


#ifndef PRETTYPRINT_H
#define PRETTYPRINT_H

#include <stddef.h>
#include <stdbool.h>

#include "callback.h"

int pp_para_gory(const Statement *stmt,
    const Parameter *p,
    bool before,
    void *user_ptr);

int pp_before_gory(const Statement *stmt, void *user_ptr);


int pp_before_after_end_gory(const Statement *stmt, void *user_ptr);

int pp_after_gory(const Statement *stmt,
    const struct sqlca *s,
    const struct oraca *oraca,
    void *user_ptr);

// forward decl
struct SQL_Token;

struct SQL_PP_State {
  struct SQL_Token *tokens;
  size_t tokens_size;
  size_t tokens_pos;
};
typedef struct SQL_PP_State SQL_PP_State;

int pp_before_sql(const Statement *stmt, void *user_ptr);
int pp_before_after_end_sql(const Statement *stmt, void *user_ptr);
int pp_after_sql(const Statement *stmt,
    const struct sqlca *s,
    const struct oraca *oraca,
    void *user_ptr);
int pp_para_sql(const Statement *stmt,
    const Parameter *p,
    bool before,
    void *user_ptr);

#endif
