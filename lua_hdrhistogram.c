/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */                                   
/* vim: set ts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/** @brief Lua HdrHistogram implementation @file */

#include <stdlib.h>
#include <string.h>
 
#include <lua.h>
#include <lauxlib.h>
 
#include "hdr_histogram.h"

#ifdef LUA_SANDBOX
#include "luasandbox_output.h"
#include "luasandbox_serialize.h"
#endif

static const char* LHDR = "lua.hdrhistogram"; 

typedef struct {
  struct hdr_histogram *hdr;
} lhdr_ud_t;

static int lhdr_new(lua_State* lua)
{
  lhdr_ud_t* ud = (lhdr_ud_t*)lua_newuserdata(lua, sizeof(*ud));
  ud->hdr = NULL;

  int n = lua_gettop(lua);
  luaL_argcheck(lua, n == 3, 0, "incorrect number of arguments");

  int64_t  lowest_trackable_value  = luaL_checknumber(lua, 1);
  int64_t  highest_trackable_value = luaL_checknumber(lua, 2);
  int      significant_figures     = luaL_checkint(lua, 3);
  luaL_argcheck(lua, lowest_trackable_value >= 1,
                1, "lowest trackable value must be >= 1");
  luaL_argcheck(lua, lowest_trackable_value * 2 > highest_trackable_value,
                2, "highest trackable value must be > lowest*2");
  luaL_argcheck(lua, 0 < significant_figures &&
                     6 > significant_figures,
                3, "significant figures must be 1-5");

  int r = hdr_init(lowest_trackable_value,
                   highest_trackable_value,
                   significant_figures,
                   &ud->hdr);
  if (r != 0 || ud->hdr == NULL)  {
    luaL_error(lua, "failed to create hdrhistogram");
  }

  luaL_getmetatable(lua, LHDR);
  lua_setmetatable(lua, -2);

  return 1;
}

static lhdr_ud_t* check_hdrhistogram(lua_State* lua, int args)
{
  lhdr_ud_t* ud = luaL_checkudata(lua, 1, LHDR);
  if (!ud->hdr) luaL_error(lua, "called into dead object");
  luaL_argcheck(lua, ud != NULL, 1, "invalid userdata type");
  luaL_argcheck(lua, args == lua_gettop(lua), 0,
                "incorrect number of arguments");
  return (lhdr_ud_t*)ud;
}

static int lhdr_memsize(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  size_t mem_size = 0;
  mem_size = hdr_get_memory_size(ud->hdr);
  lua_pushnumber(lua, mem_size);
  return 1;
}

static int lhdr_count(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  lua_pushnumber(lua, ud->hdr->total_count);
  return 1;
}

static int lhdr_record_value(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 2);
  int64_t value = luaL_checknumber(lua, 2);
  bool success = hdr_record_value(ud->hdr, value);
  lua_pushboolean(lua, success);
  return 1;
}
 
static int lhdr_min(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  int64_t min = 0;
  if (ud->hdr->total_count > 0) {
    min = hdr_min(ud->hdr);
  }
  lua_pushnumber(lua, min);
  return 1;
}

static int lhdr_max(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  int64_t max = 0;
  if (ud->hdr->total_count > 0) {
    max = hdr_max(ud->hdr);
  }
  lua_pushnumber(lua, max);
  return 1;
}

static int lhdr_mean(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  double mean = 0.0;
  if (ud->hdr->total_count > 0) {
    mean = hdr_mean(ud->hdr);
  }
  lua_pushnumber(lua, mean);
  return 1;
}

static int lhdr_stddev(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  double stddev = 0.0;
  if (ud->hdr->total_count > 0) {
    stddev = hdr_stddev(ud->hdr);
  }
  lua_pushnumber(lua, stddev);
  return 1;
}

static int lhdr_percentile(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 2);
  double percentile = luaL_checknumber(lua, 2);
  int64_t value = 0;
  value = hdr_value_at_percentile(ud->hdr, percentile);
  lua_pushnumber(lua, value);
  return 1;
}

static int lhdr_merge(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 2);
  lhdr_ud_t* udf = luaL_checkudata(lua, 2, LHDR);
  int64_t dropped = 0;
  dropped = hdr_add(ud->hdr, udf->hdr);
  lua_pushnumber(lua, dropped);
  return 1;
}

static int lhdr_reset(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  hdr_reset(ud->hdr);
  return 0;
}

static int lhdr_version(lua_State* lua)
{
  lua_pushstring(lua, DIST_VERSION);
  return 1;
}

static int lhdr_gc(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);
  hdr_reset(ud->hdr);
  ud->hdr = NULL;
  return 0;
}

static int lhdr_tostring(lua_State* lua)
{
  lhdr_ud_t* ud = check_hdrhistogram(lua, 1);

  lua_pushfstring(lua, "lowest_trackable_value:  %d\n"
                       "highest_trackable_value: %d\n"
                       "significant_figures:     %d\n"
                       "unit_magnitude:          %d\n"
                       "total_count:             %d\n"
                       "bucket_count:            %d\n"
                       "sub_bucket_count:        %d\n"
                       "counts_len:              %d",
                        ud->hdr->lowest_trackable_value,
                        ud->hdr->highest_trackable_value,
                        ud->hdr->significant_figures,
                        ud->hdr->unit_magnitude,
                        ud->hdr->total_count,
                        ud->hdr->bucket_count,
                        ud->hdr->sub_bucket_count,
                        ud->hdr->counts_len);
//  hdr_percentiles_print(ud->hdr, stdout, 5, 1.0, CLASSIC);
  return 1;
}

static const struct luaL_Reg lhdr_functions[] = {
  { "new", lhdr_new               },
  { "version", lhdr_version       },
  { NULL,  NULL                   }
};
 
static const struct luaL_Reg lhdr_methods[] = {
  { "reset",       lhdr_reset     },
  { "memsize",     lhdr_memsize   },
  { "count",       lhdr_count     },
  { "record",      lhdr_record_value },
  { "min",         lhdr_min       },
  { "max",         lhdr_max       },
  { "mean",        lhdr_mean      },
  { "stddev",      lhdr_stddev    },
  { "percentile",  lhdr_percentile},
  { "merge",       lhdr_merge     },
  { "__gc",        lhdr_gc        },
  { "__tostring",  lhdr_tostring  },
  { NULL,          NULL           },
};
 
int luaopen_hdrhistogram(lua_State* lua)
{
  luaL_newmetatable(lua, LHDR);
  lua_pushvalue(lua, -1);
  lua_setfield(lua, -2, "__index");
  luaL_register(lua, NULL, lhdr_methods);
  luaL_register(lua, "hdrhistogram", lhdr_functions);
  return 1;
}
