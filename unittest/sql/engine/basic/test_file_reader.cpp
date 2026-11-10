/* Copyright (c) 2026 OceanBase. Licensed under the Apache License, Version 2.0. */
// Standalone regression for the streaming reader; no database runtime required.
#include "sql/engine/basic/file_sql/file_reader.h"
#include <cassert>
#include <fstream>
using namespace oceanbase::sql::file_sql;
static void write(const std::string &path, const std::string &data) {
  std::ofstream f(path); f << data; f.close();
}
int main() {
  char dir[] = "/tmp/file-reader-XXXXXX"; assert(mkdtemp(dir));
  std::string path=std::string(dir)+"/test.csv";
  write(path,"id,id,note,day\r\n1,2,\"hello,\nworld\",2024-02-29\r\n3,4,\"say \"\"hi\"\"\",\r\n");
  Reader reader; Options o; o.path=path;
  assert(reader.open(o,dir));assert(reader.columns.size()==4);assert(reader.columns[1].name=="id__2");
  assert(reader.columns[3].type==Type::DATE&&reader.columns[3].nullable);
  std::vector<Cell> row;bool end;assert(reader.next(row,end)&&!end&&row[2].text=="hello,\nworld");
  assert(reader.next(row,end)&&!end&&row[2].text=="say \"hi\""&&row[3].null_value);
  assert(reader.next(row,end)&&end);assert(reader.rescan());assert(reader.next(row,end)&&row[0].text=="1");
  Reader decoded;assert(decode(encode(reader),decoded));assert(decoded.open(decoded.options,dir,false));
  write(path,"id,id,note,day\n9,8,changed,2024-01-01\n");assert(!reader.next(row,end));assert(!decoded.rescan());
  write(path,"a\n\"broken\n");assert(!reader.open(o,dir));
  path=std::string(dir)+"/test.jsonl";o.path=path;o.format="json";
  write(path,"{\"id\":1,\"name\":\"a\\u4e2d\"}\n{\"id\":2,\"score\":1.5}\n");
  assert(reader.open(o,dir));assert(reader.columns.size()==3);assert(reader.columns[1].nullable);
  assert(reader.next(row,end)&&row[1].text=="a中"&&row[2].null_value);
  assert(reader.next(row,end)&&row[1].null_value&&row[2].text=="1.5");
  write(path,"{\"x\":{\"nested\":1}}\n");assert(!reader.open(o,dir));
  assert(infer(Cell("9223372036854775808"))==Type::VARCHAR);
  assert(infer(Cell("2023-02-29"))==Type::VARCHAR);
  assert(widen(Type::DOUBLE,Type::BIGINT)==widen(Type::BIGINT,Type::DOUBLE));
  std::string resolved,error;assert(!canonical("/etc/passwd",dir,resolved,error));
  unlink(path.c_str());unlink((std::string(dir)+"/test.csv").c_str());rmdir(dir);
}
