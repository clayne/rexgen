/*
 *  Copyright (C) 2012-2013  Jan Starke <rexgen@outofbed.org>
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include <librexgen/librexgen_lua.h>
#include <librexgen/librexgen.h>
#include <librexgen/unicode.h>
#include <librexgen/iterator/iterator.h>
#include <librexgen/regex/regex.h>
#include <librexgen/simplestring.h>
#include <librexgen/rexgen_options.h>
#include <uniconv.h>
#include <vector>

#if REXGEN_DEBUG == 1
#include <execinfo.h>
#include <signal.h>
#include <librexgen/stacktrace.h>
#endif

extern "C" {
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
}

#include <string>

#ifdef YYDEBUG
extern int rexgen_debug;
#endif

static const int BUFFER_SIZE = 4096;

static const luaL_Reg rexgen_lib[] = {
  { "parse_regex",      rexgen_parse_regex },
  { "get_syntax_tree",  rexgen_get_syntax_tree },
  {NULL, NULL}
};

#ifdef REXGEN_DEBUG
#if REXGEN_DEBUG == 1
static void handler(int sig) {
  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  print_stacktrace();
  exit(1);
}
#endif
#endif

extern "C"
int luaopen_rexgen(lua_State* L) {
#if YYDEBUG == 1
  rexgen_debug = 1;
#endif

  luaL_newlib(L, rexgen_lib);
#ifdef REXGEN_DEBUG
#if REXGEN_DEBUG == 1
  //signal(SIGSEGV, handler);
  signal(SIGABRT, handler);
#endif
#endif
  return 1;
}

extern "C"
int rexgen_iter(lua_State* L) {
  Iterator *iter =
    * reinterpret_cast<Iterator**>(lua_touserdata(L, lua_upvalueindex(1)));
  if (iter->hasNext()) {
    iter->next();
    rexgen_value(L, iter);
    return 1;
  } else {
    return 0;
  }
}

extern "C"
int rexgen_parse_regex(lua_State* L) {
  SimpleString xml;
  RexgenOptions options;
  if (lua_isboolean(L, 2)) {
    options.ignore_case = lua_toboolean(L, 2);
  } else {
    options.ignore_case = false;
  }

  Regex* re = parse_regex(luaL_checklstring(L, 1, NULL), options);

  Iterator** iter;
  iter = reinterpret_cast<Iterator**>(lua_newuserdata(L, sizeof(*iter)));
  luaL_getmetatable(L, "rexgen.iter");
  lua_setmetatable(L, -2);
  *iter = re->iterator();
  lua_pushcclosure(L, rexgen_iter, 1);

  re->appendRawValue(xml);
  xml.terminate();
  push_utf8_string(L, xml);

  return 2;
}

extern "C"
int rexgen_value(lua_State* L, const Iterator* iter) {
  SimpleString buffer;

  iter->value(buffer);
  buffer.terminate();
  push_utf8_string(L, buffer);

  return 1;
}

extern "C"
int rexgen_get_syntax_tree(lua_State* L) {
  SimpleString xml;
  RexgenOptions options;
  Regex* re = parse_regex(luaL_checklstring(L, 1, NULL), options);
  re->appendRawValue(xml);
  push_utf8_string(L, xml);
  return 1;
}

void push_utf8_string(lua_State* L, const SimpleString& str) {
  lua_pushlstring(L, (const char*)str.__get_buffer_address(), str.__get_buffer_size());
}
