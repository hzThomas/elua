// Interface with core services

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
#include "legc.h"
#include "platform_conf.h"
#include "linenoise.h"
#include "memtrace.h"
#include <string.h>

#if defined( USE_GIT_REVISION )
#include "git_version.h"
#else
#include "version.h"
#endif

#ifdef BUILD_UMON
#include "umon.h"
#endif

// Lua: elua.egc_setup( mode, [ memlimit ] )
static int elua_egc_setup( lua_State *L )
{
  int mode = luaL_checkinteger( L, 1 );
  unsigned memlimit = 0;

  if( lua_gettop( L ) >= 2 )
    memlimit = ( unsigned )luaL_checkinteger( L, 2 );
  legc_set_mode( L, mode, memlimit );
  return 0;
}

// Lua: elua.version()
static int elua_version( lua_State *L )
{
  lua_pushstring( L, ELUA_STR_VERSION );
  return 1;
}

// Lua: elua.save_history( filename )
// Only available if linenoise support is enabled
static int elua_save_history( lua_State *L )
{
#ifdef BUILD_LINENOISE
  const char* fname = luaL_checkstring( L, 1 );
  int res;

  res = linenoise_savehistory( LINENOISE_ID_LUA, fname );
  if( res == 0 )
    printf( "History saved to %s.\n", fname );
  else if( res == LINENOISE_HISTORY_NOT_ENABLED )
    printf( "linenoise not enabled for Lua.\n" );
  else if( res == LINENOISE_HISTORY_EMPTY )
    printf( "History empty, nothing to save.\n" );
  else
    printf( "Unable to save history to %s.\n", fname );
  return 0;
#else // #ifdef BUILD_LINENOISE
  return luaL_error( L, "linenoise support not enabled." );
#endif // #ifdef BUILD_LINENOISE
}

// Lua: elua.c_stack_trace()
#ifdef BUILD_UMON
static int elua_c_stack_trace( lua_State *L )
{
  int stat = umon_trace_start();
  umon_print_stack_trace();
  umon_trace_end( stat );
  return 0;
}
#endif

#ifdef ENABLE_MEM_TRACER
static int elua_mt_start( lua_State *L )
{
  const char *msg = luaL_optstring( L, 1, "" );

  mt_start( msg );
  return 0;
}

static int elua_mt_stop( lua_State *L )
{
  mt_stop();
  return 0;
}
#endif

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE elua_map[] =
{
  { LSTRKEY( "egc_setup" ), LFUNCVAL( elua_egc_setup ) },
  { LSTRKEY( "version" ), LFUNCVAL( elua_version ) },
  { LSTRKEY( "save_history" ), LFUNCVAL( elua_save_history ) },
#ifdef BUILD_UMON
  { LSTRKEY( "c_stack_trace" ), LFUNCVAL( elua_c_stack_trace ) },
#endif
#ifdef ENABLE_MEM_TRACER
  { LSTRKEY( "mt_start" ), LFUNCVAL( elua_mt_start ) },
  { LSTRKEY( "mt_stop" ), LFUNCVAL( elua_mt_stop ) },
#endif
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "EGC_NOT_ACTIVE" ), LNUMVAL( EGC_NOT_ACTIVE ) },
  { LSTRKEY( "EGC_ON_ALLOC_FAILURE" ), LNUMVAL( EGC_ON_ALLOC_FAILURE ) },
  { LSTRKEY( "EGC_ON_MEM_LIMIT" ), LNUMVAL( EGC_ON_MEM_LIMIT ) },
  { LSTRKEY( "EGC_ALWAYS" ), LNUMVAL( EGC_ALWAYS ) },
#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_elua( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else
  luaL_register( L, AUXLIB_ELUA, elua_map );
  MOD_REG_NUMBER( L, "EGC_NOT_ACTIVE", EGC_NOT_ACTIVE );
  MOD_REG_NUMBER( L, "EGC_ON_ALLOC_FAILURE", EGC_ON_ALLOC_FAILURE );
  MOD_REG_NUMBER( L, "EGC_ON_MEM_LIMIT", EGC_ON_MEM_LIMIT );
  MOD_REG_NUMBER( L, "EGC_ALWAYS", EGC_ALWAYS );
  return 1;
#endif
}
