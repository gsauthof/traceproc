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


#ifndef PROC_UTIL_H
#define PROC_UTIL_H

struct oraca;

void proc_error(int errorcode, struct oraca *o, const char *extra_msg, const char *func,
    const char *filename, int line);

int proc_connect();
int proc_disconnect_commit();
int proc_disconnect_rollback();


// call before connect!
// and use:
//  EXEC ORACLE OPTION (ORACA=YES);
//  EXEC SQL INCLUDE ORACA; 
#define proc_init_oraca(A) \
  do {    \
    oraca.oradbgf  = 1; /* enable debug features */   \
    oraca.orahchf = 1;  /* enable heap checking */  \
    oraca.orastxtf = 3; /* save text of all SQL stmts */    \
    /*oraca.oracchf  = 1; */ /* enable cursor cache stats */ \
  } while(0)



#endif
