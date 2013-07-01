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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include "ora_util.h"
#include "ret_check.h"

int ora_init_vars2(char *username, char *password, char *dbspec,
    size_t size)
{
  if (!size)
    return -1;
  const char *e = getenv("ORACLE_USER");
  if (e)
    strncpy(username, e, size-1);
  else
    return -2;
  e = getenv("ORACLE_PASS");
  if (e)
    strncpy(password, e, size-1);
  else
    return -2;
  e = getenv("ORACLE_SID");
  if (e)
    strncpy(dbspec, e, size-1);
  else
    return -2;
  return 0;
}

int ora_init_vars(Ora_Connect *pc)
{
  return ora_init_vars2(pc->username, pc->password, pc->dbspec, 31);
}


int ora_reset_signals()
{
  struct sigaction act = {0};
  act.sa_handler = SIG_DFL;
  int ret = 0;
  static const int sigs[] = {
    //SIGINT,
    SIGQUIT,
    SIGILL,
    SIGTRAP,
    SIGABRT,
    SIGBUS,
    SIGFPE,
    SIGSEGV,
    0
  };
  const int *s = sigs;
  for (; *s; ++s) {
    ret = sigaction(*s, &act, 0);
    IFTRUERET(ret, 0, 1);
  }
  return 0;
}

struct Sig_Str  { int sig; const char *str; };
typedef struct Sig_Str Sig_Str;
Sig_Str sig_str[] =
{
  { SIGHUP, "SIGHUP" },
  { SIGINT, "SIGINT" },
  { SIGQUIT, "SIGQUIT" },
  { SIGILL, "SIGILL" },
  { SIGABRT, "SIGABRT" },
  { SIGFPE, "SIGFPE" },
  { SIGKILL, "SIGKILL" },
  { SIGSEGV, "SIGSEGV" },
  { SIGPIPE, "SIGPIPE" },
  { SIGALRM, "SIGALRM" },
  { SIGTERM, "SIGTERM" },
  { SIGUSR1, "SIGUSR1" },
  { SIGUSR2, "SIGUSR2" },
  { SIGCHLD, "SIGCHLD" },
  { SIGCONT, "SIGCONT" },
  { SIGSTOP, "SIGSTOP" },
  { SIGTSTP, "SIGTSTP" },
  { SIGTTIN, "SIGTTIN" },
  { SIGTTOU, "SIGTTOU" },
  { SIGBUS, "SIGBUS" },
  { SIGPOLL, "SIGPOLL" },
  { SIGPROF, "SIGPROF" },
  { SIGSYS, "SIGSYS" },
  { SIGTRAP, "SIGTRAP" },
  { SIGURG, "SIGURG" },
  { SIGVTALRM, "SIGVTALRM" },
  { SIGXCPU, "SIGXCPU" },
  { SIGXFSZ, "SIGXFSZ" },
  { SIGWINCH, "SIGWINCH" },
  { SIGPWR, "SIGPWR" },
  { SIGSTKFLT, "SIGSTKFLT" },
  { 0, 0 }
};

static const char *sig_to_str(int i)
{
  const Sig_Str *s = sig_str;
  for (; s->sig; ++s) {
    if (s->sig == i) {
      return s->str;
    }
  }
  return "UNK";
}

static void breakme(void (*fp)(int, siginfo_t *, void *))
{
  (void)fp;
}

int ora_print_signals(const char *label)
{
  void (*fp1)(int, siginfo_t *, void *);
  void (*fp2)(int, siginfo_t *, void *);


  fprintf(stderr, "# Signals - %s\n", label);
  fprintf(stderr, "#%5s | %15s | %20s | %10s | %7s | %7s\n", "No", "NAME", "Pointer", "SA_SIGINFO", "SIG_DFL", "SIG_IGN");
  int i = 0;
  for (i = 1; i<32; ++i) {
    struct sigaction act = {0};
    int ret = sigaction(i, 0, &act);
    IFTRUERET(ret, strerror(errno), -1);
    fprintf(stderr, "%6d | %15s | %20p | %10s | %7s | %7s\n", i, sig_to_str(i), act.sa_sigaction, act.sa_flags & SA_SIGINFO ? "true" : "false", act.sa_handler == SIG_DFL ? "true" : "false", act.sa_handler == SIG_IGN ? "true" : "false");
    if (i == 2)
      fp1 = act.sa_sigaction;
    if (i == 3)
      fp2 = act.sa_sigaction;
  }
  fprintf(stderr, "\n\n");
  breakme(fp1);
  breakme(fp2);
  fprintf(stderr, "\n\n");
  return 0;
}

