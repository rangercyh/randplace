#include <lua.h>
#include <lauxlib.h>
#include "rand_place.h"

#define MT_NAME ("_randplace_metatable")

static int
ldighole(lua_State *L) {
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int w = luaL_checkinteger(L, 4);
    int h = luaL_checkinteger(L, 5);
    if (w <= 0 || h <= 0) {
        luaL_error(L, "width or height(%d,%d) can not be < 0", w, h);
    }
    lua_getiuservalue(L, 1, 1);
    lua_pushinteger(L, dig_hole(lua_touserdata(L, -1), x, y, w, h));
    return 1;
}

static int
lrandplace(lua_State *L) {
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    if (w <= 0 || h <= 0) {
        luaL_error(L, "width or height(%d,%d) can not be < 0", w, h);
    }
    lua_getiuservalue(L, 1, 1);
    struct map *m = lua_touserdata(L, -1);
    if (rand_place(m, w, h)) {
        lua_pushinteger(L, m->place[0]);
        lua_pushinteger(L, m->place[1]);
        return 2;
    }
    lua_pushnil(L);
    return 1;
}

static int
gc(lua_State *L) {
    lua_getiuservalue(L, 1, 1);
    if (lua_islightuserdata(L, -1) == 1) {
        destory(lua_touserdata(L, -1));
    }
    return 0;
}

static int
lmetatable(lua_State *L) {
    if (luaL_newmetatable(L, MT_NAME)) {
        luaL_Reg l[] = {
            {"dighole", ldighole},
            {"randplace", lrandplace},
            { NULL, NULL }
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");
    }
    return 1;
}

static int
lnew(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    if (w <= 0 || h <= 0) {
        luaL_error(L, "width or height(%d,%d) can not be < 0", w, h);
    }
    lua_newuserdatauv(L, 0, 1);
    struct map *m = create_map(x, y, w, h);
    lua_pushlightuserdata(L, m);
    lua_setiuservalue(L, -2, 1);
    lmetatable(L);
    lua_setmetatable(L, -2);
    return 1;
}

LUAMOD_API int
luaopen_randplace(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "new", lnew },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}

