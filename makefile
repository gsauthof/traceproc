

.PHONY: all

all: examples libtraceproc.so unittest/main

CPPFLAGS = -I$(ORACLE_HOME)/precomp/public
CFLAGS = -g -Wall -Wno-missing-braces -Wno-unused-variable -std=gnu99
CXXFLAGS = -g -Wall -Wno-missing-braces

#LDFLAGS64 =

CFLAGS_PTHREAD = -pthread
LDFLAGS_PTHREAD = -pthread

#CPPFLAGS_CK =
#LDFLAGS_CK =

LDFLAGS = $(LDFLAGS64)

SHAREDFLAGS = -fpic

PROC_SQLCHECK = full

# XXX include
PROC = proc
PROCFLAGS = lines=yes \
  code=ANSI_C \
  sqlcheck=$(PROC_SQLCHECK) \
  sys_include=$(ORACLE_HOME)/precomp/public \
  sys_include=/usr/lib/gcc/x86_64-redhat-linux/4.4.7/include \
  sys_include=/usr/include \
  sys_include=/usr/include/linux \
  userid=$(ORACLE_USER)/$(ORACLE_PASS)@$(ORACLE_SID)
  #parse=none \

%.c: %.pc
	$(PROC) $(PROCFLAGS) $(CPPFLAGS:-I%=include=%) iname=$< oname=$@

#wrap.po: CFLAGS += -std=gnu99

%.po: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SHAREDFLAGS) -c -o $@ $<


OBJ_LIBTRACEPROC = wrap.po parsesql.po timespec.po prettyprint.po trace.po ocitrace.po stats.po trap.po

LINK.so = $(CC) -shared $(SHAREDFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

libtraceproc.so: LDFLAGS = $(LDFLAGS64)
libtraceproc.so: LDLIBS = -ldl -lrt

libtraceproc.so: $(OBJ_LIBTRACEPROC)
	$(LINK.so)

TEMP += $(OBJ_LIBTRACEPROC) libtraceproc.so

libtraceproc_dummy.so: LDLIBS =
libtraceproc_dummy.so: LDFLAGS = $(LDFLAGS64)

libtraceproc_dummy.so: callback_dummy.po
	$(LINK.so)

TEMP += libtraceproc_dummy.so callback_dummy.po

parsesql: CFLAGS += -DTEST_PARSESQL

TEMP += parsesql.o parsesql

###########################################################################
# Unittests
###########################################################################

OBJ_UNITTEST = unittest/main.o \
	       unittest/test_timespec.o \
	       unittest/test_parsesql.o \
	       unittest/test_trace.o

OBJ_UNITTEST += timespec.o \
		parsesql.o \
		trace.o

unittest/%.o: CPPFLAGS += -I. $(CPPFLAGS_CK)

unittest/%: LDFLAGS += $(LDFLAGS_CK)

unittest/test_trace.o: CXXFLAGS += $(CFLAGS_PTHREAD)

unittest/main: LDLIBS += -lcheck

# for trace.o
unittest/main: LDLIBS += -lrt -lm

unittest/main: LDFLAGS += $(LDFLAGS_PTHREAD)

unittest/main: $(OBJ_UNITTEST)

# relying on implicit GNU make rule:
# %: %.o
#	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

TEMP += $(OBJ_UNITTEST) unittest/main

OBJ_PROC_UNITTEST = unittest/proc.o \
		    unittest/test_proc_basic.o
OBJ_PROC_UNITTEST += unittest/basic.o
OBJ_PROC_UNITTEST += proc_util.o ora_util.o 

TEMP += proc_util.c

unittest/basic.o: CPPFLAGS += -Iunittest

unittest/proc: $(OBJ_PROC_UNITTEST) libtraceproc_dummy.so libtraceproc.so
	$(LINK.o) $(filter %.o,$^) $(LDLIBS) -o $@

unittest/proc: LDFLAGS = -L$(PWD) -Wl,-R,$(PWD) $(LDFLAGS64) $(LDFLAGS_CK)
#unittest/proc: LDLIBS += -ltraceproc

unittest/proc: LDFLAGS += -L$(ORACLE_HOME)/lib -Xlinker -R$(ORACLE_HOME)/lib
unittest/proc: LDLIBS += -lclntsh
unittest/proc: LDLIBS += -lcheck

unittest/proc: LDLIBS += -ltraceproc_dummy

# LD_PRELOAD=./libtraceproc.so

TEMP += $(OBJ_PROC_UNITTEST) unittest/proc


.PHONY: testunitmain

testunitmain: unittest/main
	./$<

.PHONY: testunitproc

testunitproc: unittest/proc libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="-intercept -f/dev/null" ./$<

.PHONY: testunit

testunit: testunitmain testunitproc

###########################################################################
# Module Tests 
###########################################################################

.PHONY: genmodsql

MODSQL_OPTS = -intercept -notime -nogory -sql -nostats 
MODGORY_OPTS = -intercept -notime -gory -nosql -nostats 
MODOCI_OPTS = -nointercept -notime -gory -nosql -nostats -oci

genmodsql: example/main libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="$(MODSQL_OPTS) -fmodtest/ref/sql" ./example/main

.PHONY: testmodsql

DIFF = diff
SED =sed

testmodsql: example/main libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="$(MODSQL_OPTS) -fmodtest/out/sql" ./example/main
	$(DIFF) -I '^TRACEPROC_OPTIONS' -u modtest/ref/sql modtest/out/sql

.PHONY: genmodgory

genmodgory: example/main libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="$(MODGORY_OPTS) -fmodtest/ref/gory" ./example/main

.PHONY: testmodgory

testmodgory: example/main libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="$(MODGORY_OPTS) -fmodtest/out/gory" ./example/main
	$(DIFF) -I '^TRACEPROC_OPTIONS' -u modtest/ref/gory modtest/out/gory

.PHONY: genmodoci

genmodoci: example/main libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="$(MODOCI_OPTS) -fmodtest/ref/oci" ./example/main
	$(SED) -i s'/0x[0-9a-f]\+//g' modtest/ref/oci

.PHONY: testmodoci

testmodoci: example/main libtraceproc.so
	LD_PRELOAD=./libtraceproc.so TRACEPROC_OPTIONS="$(MODOCI_OPTS) -fmodtest/out/oci" ./example/main
	$(SED) -i s'/0x[0-9a-f]\+//g' modtest/out/oci
	$(DIFF) -I '^TRACEPROC_OPTIONS' -u modtest/ref/oci modtest/out/oci

.PHONY: genmod

genmod: genmodsql genmodgory genmodoci

.PHONY: testmod

testmod: testmodsql testmodgory testmodoci

###########################################################################
# Test programs
###########################################################################

.PHONY: examples

examples: example/main example/massinsert example/prefetch example/signal


#prefetch massinsert example: 
example/occi/% example/otl/% example/oci/% example/%: LDFLAGS = $(LDFLAGS64) -L$(ORACLE_HOME)/lib -Xlinker -R$(ORACLE_HOME)/lib

example/occi/% example/otl/% example/oci/% example/%: LDLIBS = -lclntsh

example/occi/%.o example/otl/%.o example/oci/%.o example/%.o: CPPFLAGS += -I.

example/occi/%.o ocitrace.po example/otl/%.o oci_util.o example/oci/%.o: CPPFLAGS += -I$(ORACLE_HOME)/rdbms/public


example/main: PROC_SQLCHECK = syntax

example/main: example/main.o proc_util.o ora_util.o 

TEMP += example/main.c example/main.o example/main


example/massinsert.c: PROC_SQLCHECK = syntax

example/massinsert: example/massinsert.o proc_util.o ora_util.o 

TEMP += example/massinsert.c example/massinsert.o example/massinsert

example/massinsert_impl: PROCFLAGS += MAX_ROW_INSERT=1000

example/massinsert_impl: example/massinsert
	cp $< $@

TEMP += example/massinsert_impl

example/prefetch: PROC_SQLCHECK = syntax

example/prefetch: example/prefetch.o proc_util.o ora_util.o 

TEMP += example/prefetch.c example/prefetch.o example/prefetch

example/arrayfetch: example/arrayfetch.o proc_util.o ora_util.o

TEMP += example/arrayfetch.c example/arrayfetch.o example/arrayfetch

example/ptr: PROC_SQLCHECK = syntax
example/ptr: example/ptr.o proc_util.o ora_util.o

TEMP += example/ptr.c example/ptr.o example/ptr

example/dynamic: PROC_SQLCHECK = syntax
example/dynamic: example/dynamic.o proc_util.o ora_util.o

TEMP += example/dynamic.c example/dynamic.o example/dynamic

example/signal: example/signal.o proc_util.o ora_util.o

TEMP += example/signal.c example/signal.o example/signal

example/oci/main: example/oci/main.o ora_util.o oci_util.o

TEMP += example/oci/main.o example/oci/main oci_util.o


example/occi/% example/otl/fetch: CC = $(CXX)

example/otl/fetch: example/otl/fetch.o ora_util.o

OTL_PREFIX = $(HOME)/src/otl

example/otl/%.o: CPPFLAGS += -I$(OTL_PREFIX)

TEMP += example/otl/fetch.o example/otl/fetch

example/occi/%: LDLIBS += -locci

example/occi/fetch: example/occi/fetch.o ora_util.o

TEMP +=  example/occi/fetch.o example/occi/fetch

###########################################################################
# Cleanup
###########################################################################

.PHONY: clean

clean:
	rm -rf $(TEMP)


###########################################################################
# Don't remove intermediate files
###########################################################################

.SECONDARY:


