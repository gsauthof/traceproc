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



static const char *sql_fn_code_str[] =
{
  "0CODE",          //00
  "CREATE TABLE", // 1
  "SET ROLE", // 2
  "INSERT", // 3
  "SELECT", // 4
  "UPDATE", // 5
  "DROP ROLE", // 6
  "DROP VIEW", // 7
  "DROP TABLE", // 8
  "DELETE", // 9
  "CREATE VIEW", // 10
  "DROP USER", // 11
  "CREATE ROLE", // 12
  "CREATE SEQUENCE", // 13
  "ALTER SEQUENCE", // 14
  "15 (NOT USED) ",
  "DROP SEQUENCE", // 16
  "CREATE SCHEMA", // 17
  "CREATE CLUSTER", // 18
  "CREATE USER", // 19
  "CREATE INDEX", // 20
  "DROP INDEX", // 21
  "DROP CLUSTER", // 22
  "VALIDATE INDEX", // 23
  "CREATE PROCEDURE", // 24
  "ALTER PROCEDURE", // 25
  "ALTER TABLE", // 26
  "EXPLAIN", // 27
  "GRANT", // 28
  "REVOKE", // 29
  "CREATE SYNONYM", // 30
  "DROP SYNONYM", // 31
  "ALTER SYSTEM SWITCH LOG", // 32
  "SET TRANSACTION", // 33
  "PL/SQL EXECUTE", // 34
  "LOCK", // 35
  "NOOP", // 36
  "RENAME", // 37
  "COMMENT", // 38
  "AUDIT", // 39
  "NO AUDIT", // 40
  "ALTER INDEX", // 41
  "CREATE EXTERNAL DATABASE", // 42
  "DROP EXTERNAL DATABASE", // 43
  "CREATE DATABASE", // 44
  "ALTER DATABASE", // 45
  "CREATE ROLLBACK SEGMENT", // 46
  "ALTER ROLLBACK SEGMENT", // 47
  "DROP ROLLBACK SEGMENT", // 48
  "CREATE TABLESPACE", // 49
  "ALTER TABLESPACE", // 50
  "DROP TABLESPACE", // 51
  "ALTER SESSION", // 52
  "ALTER USER", // 53
  "COMMIT (WORK) ", // 54
  "ROLLBACK", // 55
  "SAVEPOINT", // 56
  "CREATE CONTROL FILE", // 57
  "ALTER TRACING", // 58
  "CREATE TRIGGER", // 59
  "ALTER TRIGGER", // 60
  "DROP TRIGGER", // 61
  "ANALYZE TABLE", // 62
  "ANALYZE INDEX", // 63
  "ANALYZE CLUSTER", // 64
  "CREATE PROFILE", // 65
  "DROP PROFILE", // 66
  "ALTER PROFILE", // 67
  "DROP PROCEDURE", // 68
  "69 (NOT USED) ",
  "ALTER RESOURCE COST", // 70
  "CREATE SNAPSHOT LOG", // 71
  "ALTER SNAPSHOT LOG", // 72
  "DROP SNAPSHOT LOG", // 73
  "CREATE SNAPSHOT", // 74
  "ALTER SNAPSHOT", // 75
  "DROP SNAPSHOT", // 76
  "CREATE TYPE", // 77
  "DROP TYPE", // 78
  "ALTER ROLE", // 79
  "ALTER TYPE", // 80
  "CREATE TYPE BODY", // 81
  "ALTER TYPE BODY", // 82
  "DROP TYPE BODY", // 83
  "DROP LIBRARY", // 84
  "TRUNCATE TABLE", // 85
  "TRUNCATE CLUSTER", // 86
  "CREATE BITMAPFILE", // 87
  "ALTER VIEW", // 88
  "DROP BITMAPFILE", // 89
  "SET CONSTRAINTS", // 90
  "CREATE FUNCTION", // 91
  "ALTER FUNCTION", // 92
  "DROP FUNCTION", // 93
  "CREATE PACKAGE", // 94
  "ALTER PACKAGE", // 95
  "DROP PACKAGE", // 96
  "CREATE PACKAGE BODY", // 97
  "ALTER PACKAGE BODY", // 98
  "DROP PACKAGE BODY", // 99
  "UNK (100)", // 100
  "UNK (101)", // 101
  "UNK (102)", // 102
  "UNK (103)", // 103
  "UNK (104)", // 104
  "UNK (105)", // 105
  "UNK (106)", // 106
  "UNK (107)", // 107
  "UNK (108)", // 108
  "UNK (109)", // 109
  "UNK (110)", // 110
  "UNK (111)", // 111
  "UNK (112)", // 112
  "UNK (113)", // 113
  "UNK (114)", // 114
  "UNK (115)", // 115
  "UNK (116)", // 116
  "UNK (117)", // 117
  "UNK (118)", // 118
  "UNK (119)", // 119
  "UNK (120)", // 120
  "UNK (121)", // 121
  "UNK (122)", // 122
  "UNK (123)", // 123
  "UNK (124)", // 124
  "UNK (125)", // 125
  "UNK (126)", // 126
  "UNK (127)", // 127
  "UNK (128)", // 128
  "UNK (129)", // 129
  "UNK (130)", // 130
  "UNK (131)", // 131
  "UNK (132)", // 132
  "UNK (133)", // 133
  "UNK (134)", // 134
  "UNK (135)", // 135
  "UNK (136)", // 136
  "UNK (137)", // 137
  "UNK (138)", // 138
  "UNK (139)", // 139
  "UNK (140)", // 140
  "UNK (141)", // 141
  "UNK (142)", // 142
  "UNK (143)", // 143
  "UNK (144)", // 144
  "UNK (145)", // 145
  "UNK (146)", // 146
  "UNK (147)", // 147
  "UNK (148)", // 148
  "UNK (149)", // 149
  "UNK (150)", // 150
  "UNK (151)", // 151
  "UNK (152)", // 152
  "UNK (153)", // 153
  "UNK (154)", // 154
  "UNK (155)", // 155
  "UNK (156)", // 156
  "CREATE DIRECTORY", // 157
  "DROP DIRECTORY", // 158
  "CREATE LIBRARY", // 159
  "CREATE JAVA", // 160
  "ALTER JAVA", // 161
  "DROP JAVA", // 162
  "CREATE OPERATOR", // 163
  "CREATE INDEXTYPE", // 164
  "DROP INDEXTYPE", // 165
  "ALTER INDEXTYPE", // 166
  "DROP OPERATOR", // 167
  "ASSOCIATE STATISTICS", // 168
  "DISASSOCIATE STATISTICS", // 169
  "CALL METHOD", // 170
  "CREATE SUMMARY", // 171
  "ALTER SUMMARY", // 172
  "DROP SUMMARY", // 173
  "CREATE DIMENSION", // 174
  "ALTER DIMENSION", // 175
  "DROP DIMENSION", // 176
  "CREATE CONTEXT", // 177
  "DROP CONTEXT", // 178
  "ALTER OUTLINE", // 179
  "CREATE OUTLINE", // 180
  "DROP OUTLINE", // 181
  "UPDATE INDEXES", // 182
  "ALTER OPERATOR", // 183
  0
};

unsigned sql_fn_code_str_size = 184;

