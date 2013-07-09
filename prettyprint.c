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




#include "prettyprint.h"

#include <stdio.h>
#include <stdlib.h>

#include "foreign.h" // varchar

#include "parsesql.h"
#include "ret_check.h"
#include "trace.h"

static int pp_para_gory_value(const Statement *stmt,
    const Parameter *p,
    bool before,
    void *user_ptr)
{
  if (p->indicator == -1) {
    tprintf("      Value: |NULL|\n");
  } else  {
    switch (p->type)
    {
      case STRING :
        tprintf("      Value: |%s|\n", (const char*)p->value);
        break;
      case DOUBLE :
        if (p->length < sizeof(double))
          tprintf("      Value: |%f|\n", *(float*)p->value);
        else
          tprintf("      Value: |%f|\n", *(double*)p->value);
        break;
      case INT:
        if (p->length < sizeof(long))
          tprintf("      Value: |%d|\n", *(int*)p->value);
        else
          tprintf("      Value: |%ld|\n", *(long*)p->value);
        break;
      case UNSIGNED:
        if (p->length < sizeof(unsigned long))
          tprintf("      Value: |%u|\n", *(unsigned*)p->value);
        else
          tprintf("      Value: |%lu|\n", *(long unsigned*)p->value);
        break;
      case VARCHAR:
        {
          const varchar *v = p->value;
          tprintf("      Value: |%.*s| (%hu)\n", v->len, v->arr, v->len);
        }
        break;
      case STRING0:
        tprintf("      Value: |%s|\n", (const char*)p->value);
        break;
      case CHAR:
        tprintf("      Value: |%c|\n", *(const char*)p->value);
        break;
      default:
        tprintf("      Value pp not implemented\n");
        break;
    }
  }
  return 0;
}

int pp_para_gory(const Statement *stmt,
    const Parameter *p,
    bool before,
    void *user_ptr)
{
  if (!(!stmt->errorcode || stmt->errorcode == 1403 || stmt->errorcode == 100))
    return 0;

  if (!p->pos) {
    if (!p->iteration)
      tprintf("Iterations: %zu (of %zu)\n",
          p->iterations, stmt->iterations);
    tprintf("  Iteration: %zu\n", p->iteration);
  }
  switch (p->direction) {
    case PARA_IN:
      if (stmt->type == SELECT && p->iteration > 0)
        return 0;
      tprintf("    IN paramter");
      break;
    case PARA_OUT:
      if (before)
        return 0;
      tprintf("    OUT paramter");
      break;
  }
  tprintf(" (a=%zu)", p->pos);
  tprintf("\n");
  tprintf("      Type: %s (%d), ",
      p->type_str,
      p->type);
  if (p->type != VOID) {
    tprintf("Length: %zu, Size: %zu, Ind %d\n",
        p->length,
        p->size,
        p->indicator);
  }

  return pp_para_gory_value(stmt, p, before, user_ptr);
}

int pp_before_gory(const Statement *stmt, void *user_ptr)
{
   tprintf("===========================================================================\n");
   tprintf("stmt # %d, type %s (%d), line %zu, #para %d (in %d) (offset %d) %s %s (foff: %d)\n",
     stmt->number,
     stmt->type_str,
     stmt->type,
     stmt->line_number,
     stmt->number_of_params, stmt->number_of_params_in,
     stmt->offset,
     stmt->scroll_cursor ? "SCROLL" : "",
     stmt->type == FETCH ? stmt->fetch_type_str : "",
     stmt->fetch_offset
   );
  if (stmt->type == CONNECT)
    tprintf("MAX_ROW_INSERT=%d (max # of implicitly buffered rows on INSERT)\n", stmt->max_row_insert);
  if (stmt->type == OPEN || stmt->type == FETCH)
    tprintf("PREFETCH=%zu (# of prefetched rows)\n", stmt->prefetch);
  if (stmt->text) {
    tprintf("SQL: %s\n", stmt->text);
  }
  return 0;
}

int pp_before_after_end_gory(const Statement *stmt, void *user_ptr)
{
   tprintf("---------------------------------------------------------------------------\n");
  return 0;
}

int pp_after_gory(const Statement *stmt,
    const struct sqlca *s,
    const struct oraca *oraca,
    void *user_ptr)
{
  tprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
  tprintf("SQL-Code: %d", stmt->errorcode);
  if (stmt->errorcode) {
    tprintf(", %s", stmt->msg);
    tprintf(" (%s)", stmt->extended_msg);
    tprintf("\n");
  }
  if (!stmt->errorcode || stmt->errorcode == 1403 || stmt->errorcode == 100) {
    tprintf("\n");
  }
  return 0;
}


int pp_sql(const Statement *stmt, void *user_ptr)
{
  //tprintf("XXX: %s\n", stmt->text ? stmt->text : "");

  tprintf("-- %s:%zu\n", stmt->filename, stmt->line_number);

  if (!stmt->text) {
    if (stmt->type_str && *stmt->type_str == '0')
      tprintf("-- Unknown SQL statement type: %d\n", stmt->type);
    else
      tprintf("-- Pseudo-SQL: %s\n", stmt->type_str);
  }

  if (stmt->type == CONNECT)
     tprintf("-- MAX_ROW_INSERT=%d (max # of implicitly buffered rows on INSERTs)\n", stmt->max_row_insert);
  if (stmt->type == OPEN || stmt->type == FETCH)
    tprintf("-- PREFETCH=%zu (# of prefetched rows)\n", stmt->prefetch);

  SQL_PP_State *s = user_ptr;
  s->tokens = 0;
  s->tokens_size = 0;
  if (stmt->text) {
    int ret_tok = sql_tokenize(stmt->text, &s->tokens, &s->tokens_size);
    IFTRUERET(ret_tok, 0, -1);
  } else {
    if (stmt->type == EXECUTE && stmt->last_prepared_stmt) {
      tprintf("-- EXECUTING prepared statement:\n");
      int ret_tok = sql_tokenize(stmt->last_prepared_stmt,
          &s->tokens, &s->tokens_size);
      IFTRUERET(ret_tok, 0, -1);
    } else if (stmt->type == EXECUTE_IMMEDIATE) {
      tprintf("-- EXECUTING IMMEDIATELY statement:\n");
      int ret_tok = sql_tokenize(stmt->immediate_stmt,
          &s->tokens, &s->tokens_size);
      IFTRUERET(ret_tok, 0, -1);
    } else if (stmt->number_of_params)
      sql_pseudo_tokenize(stmt->number_of_params, &s->tokens, &s->tokens_size);
  }
  s->tokens_pos = 0;
  SQL_Token *i = s->tokens + s->tokens_pos;

  // don't print (possibly) tons of NULL fetch results
  if (stmt->type == FETCH && (stmt->errorcode == 1403 || stmt->errorcode == 100))
    return 0;

  if (s->tokens_pos < s->tokens_size && i->begin && !i->host_var) {
    tprintf("%.*s", (int) (i->end - i->begin), i->begin);
    ++s->tokens_pos;
  }

  return 0;
}

int pp_before_sql(const Statement *stmt, void *user_ptr)
{
  tprintf("\n");
  tprintf("\n");
  tprintf("-- ###########################################################################\n");
  tprintf("-- Before execution:\n");
  return pp_sql(stmt, user_ptr);
}

int pp_before_after_end_sql(const Statement *stmt, void *user_ptr)
{
  SQL_PP_State *s = user_ptr;
  if (!s->tokens)
    return 0;
  free(s->tokens);
  tprintf(";\n");
  tprintf("-- ---------------------------------------------------------------------------\n");
  return 0;
}

int pp_after_sql(const Statement *stmt,
    const struct sqlca *s,
    const struct oraca *oraca,
    void *user_ptr)
{
  tprintf("\n");
  tprintf("\n");
  tprintf("-- ===========================================================================\n");
  if (stmt->errorcode)
    tprintf("-- After execution (errorcode: %d, %s, %s):\n",
        stmt->errorcode,
        stmt->msg,
        stmt->extended_msg
        );
  else
    tprintf("-- After execution:\n");
  return pp_sql(stmt, user_ptr);
}

static int pp_para_sql_value(const Statement *stmt,
    const Parameter *p,
    bool before,
    void *user_ptr)
{
  char buffer[2048] = {0};
  if (p->indicator == -1) {
    tprintf("NULL");
  } else  {
    switch (p->type)
    {
      case STRING :
        sql_quote_str((const char*)p->value, strlen((const char*)p->value),
            buffer, 2048);
        tprintf("%s", buffer);
        break;
      case DOUBLE :
        if (p->length < sizeof(double))
          tprintf("%f", *(float*)p->value);
        else
          tprintf("%f", *(double*)p->value);
        break;
      case INT:
        if (p->length < sizeof(long))
          tprintf("%d", *(int*)p->value);
        else
          tprintf("%ld", *(long*)p->value);
        break;
      case UNSIGNED:
        if (p->length < sizeof(unsigned long))
          tprintf("%u", *(unsigned*)p->value);
        else
          tprintf("%lu", *(long unsigned*)p->value);
        break;
      case VARCHAR:
        {
          const varchar *v = p->value;
          sql_quote_str((const char*)v->arr, (size_t)v->len, buffer, 2048);
          tprintf("%s", buffer);
        }
        break;
      case STRING0:
        sql_quote_str((const char*)p->value, strlen((const char*)p->value),
            buffer, 2048);
        tprintf("%s", buffer);
        break;
      case CHAR:
        sql_quote_str((const char*)p->value, 1,
            buffer, 2048);
        tprintf("%s", buffer);
        break;
      default:
        tprintf("VAL_NOT_IMPL");
        break;
    }
  }
  return 0;
}

int pp_para_sql(const Statement *stmt,
    const Parameter *p,
    bool before,
    void *user_ptr)
{
  // XXX do something special on after and select

  SQL_PP_State *s = user_ptr;


  if (!p->pos && p->iteration && s->tokens) {
    s->tokens_pos = 0;
    tprintf(";\n");
    SQL_Token *i = s->tokens + s->tokens_pos;
    if (s->tokens_pos < s->tokens_size && i->begin && !i->host_var) {
      tprintf("%.*s", (int) (i->end - i->begin), i->begin);
      ++s->tokens_pos;
    }
  }


  if (s->tokens_pos >= s->tokens_size)
    return -1;

  SQL_Token *i = s->tokens + s->tokens_pos;

  switch (p->direction) {
    case PARA_IN:
      if (stmt->type == SELECT && p->iteration > 0 && before)
        return 0;
      break;
    case PARA_OUT:
      if (before) {
        if (s->tokens_pos < s->tokens_size && i->begin && i->host_var) {
          tprintf("%.*s", (int)(i->end - i->begin), i->begin);
          ++s->tokens_pos;
          ++i;
          if (s->tokens_pos < s->tokens_size && i->begin && !i->host_var) {
            tprintf("%.*s", (int)(i->end - i->begin), i->begin);
            ++s->tokens_pos;
          }
        }
        return 0;
      }
      break;
  }
  if (s->tokens_pos < s->tokens_size && i->begin && i->host_var) {
    int ret_val = pp_para_sql_value(stmt, p, before, user_ptr);
    IFTRUERET(ret_val, 0, ret_val);
    ++s->tokens_pos;
    ++i;
  }
  if (s->tokens_pos < s->tokens_size && i->begin && !i->host_var) {
    tprintf("%.*s", (int) (i->end - i->begin), i->begin);
    ++s->tokens_pos;
  }

  return 0;
}
