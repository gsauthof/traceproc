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




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


struct SQL_Token {
  bool host_var;
  const char *begin;
  const char *end;
};
typedef struct SQL_Token SQL_Token;


static is_hostvar_char(char c)
{
    return isalnum(c) || c == '_';
}

size_t sql_count_host_vars(const char *s)
{
  size_t ret = 0;
  const char *pos = s;
  for (;;) {
    const char *a = strchr(pos, '\'');
    const char *b = strchr(pos, ':');
    if (!b)
      break;
    if (!a || b<a) {
      ++ret;
      pos = b + 1;
      for (; is_hostvar_char(*pos); ++pos)
        ;
      if (*pos == ':')
        ++pos;
    } else {
      pos = a + 1;
      pos = strchr(pos, '\'');
      if (!pos)
        break;
      ++pos;
    }
  }
  return ret;
}

int sql_tokenize(const char *s,
    SQL_Token **tokens, size_t *size)
{
  size_t l = sql_count_host_vars(s);
  *size = l*2+2;
  *tokens = calloc(*size, sizeof(SQL_Token));
  if (!tokens)
    return -1;
  SQL_Token *out = *tokens;

  size_t n = strlen(s);
  
  const char *token_begin = s;
  const char *pos = s;
  for (;;) {
    const char *a = strchr(pos, '\'');
    const char *b = strchr(pos, ':');
    if (!b) {
      //fprintf(stderr, "|%s|\n", token_begin);
      out->begin = token_begin;
      out->end = token_begin + strlen(token_begin);
      ++out;
      return 0;
    }
    if (!a || b<a) {
      if (token_begin < b) {
        //fprintf(stderr, "|%.*s|\n", b-token_begin, token_begin);
        out->begin = token_begin;
        out->end = b;
         ++out;
      }
      const char *str_pos = b+1;
      for (;;) {
        for (; is_hostvar_char(*str_pos); ++str_pos)
          ;
        if (*str_pos != ':')
          break;
        ++str_pos;
      }
      //fprintf(stderr, "HOST|%.*s|\n", str_pos-b, b);
      out->host_var = true;
      out->begin = b;
      out->end = str_pos;
      ++out;
      token_begin = pos = str_pos;
    } else {
      const char *str_pos = a+1;
      for (;;) {
        const char *a = strchr(str_pos, '\'');
        if (!a) {
          //fprintf(stderr, "|%s|\n", token_begin);
          out->begin = token_begin;
          out->end = token_begin + strlen(token_begin);
          ++out;
          return 0;
        }
        if (!a[1]) {
          //fprintf(stderr, "|%s|\n", token_begin);
          out->begin = token_begin;
          out->end = token_begin + strlen(token_begin);
          ++out;
          return 0;
        }
        if (a[1] == '\'') {
          str_pos = a+2;
          continue;
        }
        pos = a+1;
        break;
      }
    }
  }
}


int sql_quote_str(const char *input, size_t input_size, char *output,
    size_t output_size)
{
  const char *p = input;
  *output = '\'';
  char *o = output;
  size_t written = 0;
  size_t seen = 0;
  for (const char *x = strchr(p, '\'');
      x && x-input<input_size;
      x = strchr(p, '\'')) {
    written += x-p + 2;
    if (written>=output_size)
      return -1;

    seen += x-p + 1;

    strncat(o, p, x-p);
    strcat(o, "''");
    p = x+1;
  }

  size_t n = input_size - seen;
  written += n + 1;
  if (written>=output_size)
    return -1;

  strncat(o, p, n);
  strcat(o, "'");

  return 0;
}

static const char pseudo_token[] = ", ";
static const char pseudo_host_token[] = ":XXX";
static const char pseudo_begin_token[] = "( ";
static const char pseudo_end_token[] = " )";

int sql_pseudo_tokenize(size_t n,
    SQL_Token **tokens, size_t *size)
{
  *size = 2*n+2+1;
  *tokens = calloc(*size, sizeof(SQL_Token));
  if (!tokens)
    return -1;
  SQL_Token *out = *tokens;
  out->begin = pseudo_begin_token;
  out->end = pseudo_begin_token+2;
  ++out;

  if (0<n) {
    out->host_var = true;
    out->begin = pseudo_host_token;
    out->end = pseudo_host_token+4;
    ++out;
  }
  for (size_t i = 1; i<n; ++i) {
    out->begin = pseudo_token;
    out->end = pseudo_token+2;
    ++out;
    out->host_var = true;
    out->begin = pseudo_host_token;
    out->end = pseudo_host_token+4;
    ++out;
  }
  out->begin = pseudo_end_token;
  out->end = pseudo_end_token+2;
  ++out;
  return 0;
}

// insert into example_tbl (str,n,f) values (:b0,:b1:b2,:b3) returning n into :b4:b5 



#ifdef TEST_PARSESQL

int main(int argc, char **argv)
{

  if (argc > 1 && *argv[1] == 'q') {
    char buffer[512] = {0};
    int ret = sql_quote_str(argv[2], strlen(argv[2]), buffer, 512);
    fprintf(stderr, "|%s| => |%s| (%d)\n", argv[2], buffer, ret);
    return 0;
  }

  char exp1[256] = "insert into example_tbl (str,n,f) values (:b0,:b1:b2,:b3) returning n into :b4:b5";

  if (argc > 1 && *argv[1] == 's')
    strcpy(exp1, argv[2]);

  fprintf(stderr, "Input: |%s|\n", exp1);
  SQL_Token *tokens = 0;
  size_t tokens_size = 0;
  int ret = sql_tokenize(exp1, &tokens, &tokens_size);
  if (ret)
    return 20;
  for (SQL_Token *i = tokens; i->begin; ++i)
    fprintf(stderr, "Token: |%.*s| %s\n",
        (int)(i->end-i->begin), i->begin,
        i->host_var ? "HOST_VAR" : ""
        );
  fprintf(stderr, "Count: %zu\n", sql_count_host_vars(exp1));
  return 0;


  return 0;
}

#endif
