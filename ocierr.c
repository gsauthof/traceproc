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



struct Oci_Err {
  int val;
  const char *str;
  bool call_error_get;
};
typedef struct Oci_Err Oci_Err;

static const Oci_Err oci_errs[] = {
  { OCI_SUCCESS, "OCI_SUCCESS", false },
  { OCI_SUCCESS_WITH_INFO, "OCI_SUCCESS_WITH_INFO", true },
  { OCI_NO_DATA, "OCI_NO_DATA", false },
  { OCI_ERROR, "OCI_ERROR", true },
  { OCI_INVALID_HANDLE, "OCI_INVALID_HANDLE", false },
  { OCI_NEED_DATA, "OCI_NEED_DATA", false },
  { OCI_STILL_EXECUTING, "OCI_STILL_EXECUTING", true },
  { OCI_CONTINUE, "OCI_CONTINUE", false },
  { OCI_ROWCBK_DONE, "OCI_ROWCBK_DONE", false },
  { 0, 0, false }
};
