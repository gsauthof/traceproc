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


#ifndef TRACEPROC_CALLBACK_H
#define TRACEPROC_CALLBACK_H

#include <stddef.h>
#include <stdbool.h>

enum Statement_Type {
  DROP = 1,
  DELETE = 2,
  INSERT = 3,
  SELECT = 4,
  UPDATE = 5,
  OPEN = 9, // cursor
  FETCH = 13,
  CLOSE = 15, // cursor
  PREPARE = 17, // oracle dynamic SQL
  EXECUTE = 21, // oracle dynamic SQL
  EXECUTE_IMMEDIATE =  24 ,// oracle dynamic SQL
  CONNECT =  27,
  COMMIT = 29,
  COMMIT_RELEASE = 30,
  ROLLBACK = 31,
  ROLLBACK_RELEASE = 32,
  CREATE = 44,
  OPEN_DYNAMIC = 45, // cursor, oracle dynamic SQL
  CALL = 122, // call stored procedure/function
  CONNECT_AT = 1051 // 1051 // CONNECT ... AT
};
typedef enum Statement_Type Statement_Type;

enum type_no {
  CHAR = 1,
  INT = 3,      // or long
  DOUBLE = 4 ,  // or float
  STRING0 = 5,  // ext. STRING, zero terminated, VAR is STRING
  VARCHAR = 9,
  VOID = 10,
  UNSIGNED = 68,
  STRING = 97,   // char * or char[], BLANK padded, ext. CHARZ
};
typedef enum type_no type_no;

enum Fetch_Type {
    FETCH_CURRENT = 1,
    FETCH_NEXT = 2,
    FETCH_FIRST = 4,
    FETCH_LAST = 8,
    FETCH_PRIOR = 16,
    FETCH_ABSOLUTE = 32,
    FETCH_RELATIVE = 64
};
typedef enum Fetch_Type Fetch_Type;
struct Fetch_Info {
  Fetch_Type type;
  const char *str;
};

struct Statement {
   short number; // OPEN/FETCH/CLOSE for a select have the same nr
   short offset; // into internal array, useful to identify locations
   short length ;
   short max_row_insert;
   size_t prefetch;
   const char *text;
   Statement_Type type;
   const char *type_str;
   const char *filename;
   size_t line_number;
   size_t iterations;
   short number_of_params;
   short number_of_params_in;
   bool scroll_cursor;
   Fetch_Type fetch_type;
   const char *fetch_type_str;
   int fetch_offset;
   size_t acc_fetched_rows; // sqlca.sqlerrd[2]

   const char *msg; // filled after
   const char *extended_msg;
   int errorcode;

   const char *last_prepared_stmt;
   const char *immediate_stmt;
};
typedef struct Statement Statement;

enum Para_Type { PARA_IN, PARA_OUT };
typedef enum Para_Type Para_Type;


struct Parameter {
  Para_Type direction;
  size_t iteration;
  size_t iterations;
  size_t pos; // in Statement, from 0 in every iteration
  size_t size;
  size_t ind_size;
  size_t length;
  const void *value;
  short indicator;
  type_no type;
  const char *type_str;

};
typedef struct Parameter Parameter;

// forward declarations
struct sqlca;
struct oraca;

struct Traceproc_Callbacks {
  void *user_ptr;
  int (*parameter_fn)(const Statement *stmt, const Parameter *p,
      bool before,
      void *user_ptr);
  int (*before_fn)(const Statement *stmt, void *user_ptr);
  int (*before_end_fn)(const Statement *stmt, void *user_ptr);
  int (*after_fn)(const Statement *stmt,
      const struct sqlca *s,
      const struct oraca *oraca,
      void *user_ptr);
  int (*after_end_fn)(const Statement *stmt, void *user_ptr);
};
typedef struct Traceproc_Callbacks Traceproc_Callbacks;


int traceproc_register_callbacks(
    const Traceproc_Callbacks *callbacks,
    unsigned *id);


int traceproc_unregister_callbacks(
    unsigned id
    );


#endif
