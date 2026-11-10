/* Copyright (c) 2026 OceanBase. Licensed under the Apache License, Version 2.0. */
#include "sql/engine/basic/file_sql/csv_writer.h"
#include <cassert>
using namespace oceanbase::sql::file_sql;
int main(){char dir[]="/tmp/file-copy-XXXXXX";assert(mkdtemp(dir));Options o;o.path=std::string(dir)+"/out.csv";o.nullstr="NULL";{CsvWriter w;assert(w.open(o,dir));assert(w.row({Cell("id"),Cell("note")}));assert(w.row({Cell("1"),Cell("hello,\n\"中\"")}));assert(w.row({Cell("2"),Cell()}));assert(w.finish());}{CsvWriter w;assert(!w.open(o,dir));}Reader r;assert(r.open(o,dir));std::vector<Cell> row;bool end;assert(r.next(row,end)&&row[1].text=="hello,\n\"中\"");assert(r.next(row,end)&&row[1].null_value);o.path=std::string(dir)+"/failed.csv";{CsvWriter w;assert(w.open(o,dir));assert(w.row({Cell("partial")}));}assert(access(o.path.c_str(),F_OK));unlink((std::string(dir)+"/out.csv").c_str());rmdir(dir);}
