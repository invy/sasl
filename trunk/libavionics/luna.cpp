#include "luna.h"

#include <vector>
#include <string.h>
#include <sys/types.h>
#ifndef WINDOWS
#include <dirent.h>
#else
#include <windows.h>
#endif
#include <cstdlib>


using namespace xa;


/// bitwise and
static int luaBitAnd(lua_State *L) 
{
    int n = lua_gettop(L);
    int res = 0;
    if (2 == n) 
        res = lua_tointeger(L, 1) & lua_tointeger(L, 2);
    lua_pushinteger(L, res);
    return 1;
}

/// bitwise or
static int luaBitOr(lua_State *L) 
{
    int n = lua_gettop(L);
    int res = 0;
    if (2 == n) 
        res = lua_tointeger(L, 1) | lua_tointeger(L, 2);
    lua_pushinteger(L, res);
    return 1;
}

/// bitwise xor
static int luaBitXor(lua_State *L) 
{
    int n = lua_gettop(L);
    int res = 0;
    if (2 == n) 
        res = lua_tointeger(L, 1) ^ lua_tointeger(L, 2);
    lua_pushinteger(L, res);
    return 1;
}


#ifdef WINDOWS


/// enumerate files in directory
static int luaListFiles(lua_State *L) 
{
    const char *name = lua_tostring(L, 1);
    if (! name)
        return 0;

    std::string mask = name;
    mask = mask + "\\*";

    lua_newtable(L);

    WIN32_FIND_DATA de;
    HANDLE dir = FindFirstFile(mask.c_str(), &de);

    if (dir == INVALID_HANDLE_VALUE) 
        return 1;

    int i = 1;
    do {
        lua_pushnumber(L, i);
        lua_newtable(L);
        lua_pushstring(L, "name");
        lua_pushstring(L, de.cFileName);
        lua_settable(L, -3);
        lua_pushstring(L, "type");
        lua_pushstring(L, de.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 
                "dir" : "file");
        lua_settable(L, -3);
        lua_settable(L, -3);
        i++;
    } while (FindNextFile(dir, &de));
    FindClose(dir);

    return 1;
}

#else


/// enumerate files in directory
static int luaListFiles(lua_State *L) 
{
    const char *name = lua_tostring(L, 1);
    if (! name)
        return 0;

    lua_newtable(L);

    DIR *dir = opendir(name);
    if (! dir) {
        return 1;
    }

    int i = 1;
    for (struct dirent *de = readdir(dir); de; de = readdir(dir)) {
        if ((DT_DIR == de->d_type) || (DT_LNK == de->d_type) || 
                (DT_REG == de->d_type))
        {
            lua_pushnumber(L, i);
            lua_newtable(L);
            lua_pushstring(L, "name");
            lua_pushstring(L, de->d_name);
            lua_settable(L, -3);
            lua_pushstring(L, "type");
            lua_pushstring(L, DT_DIR == de->d_type ? "dir" : "file");
            lua_settable(L, -3);
            lua_settable(L, -3);
            i++;
        }
    }
    closedir(dir);

    return 1;
}
#endif



static std::vector<double> loadLuaTable(lua_State *L)
{
    std::vector<double> values;

    /* table is in the stack at index -1 */
    lua_pushnil(L);  /* first key, table at -2 */
    while (lua_next(L, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        int index = lua_tonumber(L, -2);
        double value = lua_tonumber(L, -1);
        if (0 < index) {
            index = index - 1; // lua arrays are 1-based
            if (values.size() <= (unsigned)index)
                values.resize(index + 1, 0);
            values[index] = value;
        }
        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
    }

    return values;
}


static double tarate(double *r1, double *r2, int size, double x)
{
    int last = size - 1;

    if (x >= r1[last]) 
      return r2[last];
    else if (x <= r1[0])
      return r2[0];
   
    int k = 0;
    while ((x >= r1[k]) && (k < size))
        k++;

    double a = (r2[k-1] - r2[k]) / (r1[k-1] - r1[k]);
    double b = r2[k] - a*r1[k];              
   
    return a * x + b;
}


static int loadTable(lua_State *L)
{
    lua_pushnumber(L, 1); lua_gettable(L, 1);  // push first table
    std::vector<double> v1 = loadLuaTable(L);
    lua_pop(L, 1); // pop table
    
    lua_pushnumber(L, 2); lua_gettable(L, 1);  // push second table
    std::vector<double> v2 = loadLuaTable(L);
    lua_pop(L, 1); // pop table

    if ((v1.size() != v2.size()) || (! v1.size())) {
        lua_pushnil(L);
        return 0;
    }

    int size = v1.size();
    int vsize = size * sizeof(double);
    char *buf = (char*)lua_newuserdata(L, vsize * 2 + sizeof(int));
    memcpy(buf, &size, sizeof(int));

    double *v = &v1[0];
    memcpy(buf + sizeof(int), v, vsize);
    v = &v2[0];
    memcpy(buf + sizeof(int) + vsize, v, vsize);

    return 1;
}

static int luaTarate(lua_State *L)
{
    if ((! lua_isuserdata(L, 1)) || (! lua_isnumber(L, 2))) {
        lua_pushnil(L);
        return 1;
    }

    char* v = (char*)lua_touserdata(L, 1);
    int size = *((int*)v);
    double *v1 = (double*)(v + sizeof(int));
    double *v2 = (double*)(v + sizeof(int) + size * sizeof(double));

    lua_pushnumber(L, tarate(v1, v2, size, lua_tonumber(L, 2)));
    return 1;
}





Luna::Luna(sasl_lua_creator_callback luaCreator,
                sasl_lua_destroyer_callback luaDestroyer)
{
    this->luaDestroyer = luaDestroyer;

    if (luaCreator)
        lua = luaCreator();
    else
        lua = luaL_newstate();

    if (lua)
    {
        luaL_openlibs(lua);
        lua_register(lua, "bitand", luaBitAnd);
        lua_register(lua, "bitor", luaBitOr);
        lua_register(lua, "bitxor", luaBitXor);
        lua_register(lua, "listFiles", luaListFiles);
        lua_register(lua, "newRamzInterpolator", loadTable);
        lua_register(lua, "ramzTarate", luaTarate);

        lua_newtable(lua);
        lua_setfield(lua, LUA_REGISTRYINDEX, "xavionics");
    } else {
        std::abort();
    }
}

Luna::~Luna()
{
    if (luaDestroyer)
        luaDestroyer(lua);
    else
        lua_close(lua);
}

bool Luna::runScript(const std::string &fileName)
{
    return luaL_dofile(lua, fileName.c_str());
}

float Luna::getFieldf(int tableIdx, const std::string &field, float dflt)
{
    float v = dflt;
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        v = (float)lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return v;
}

double Luna::getFieldd(int tableIdx, const std::string &field, double dflt)
{
    lua_getfield(lua, tableIdx, field.c_str());
    double v = dflt;
    if (! lua_isnil(lua, -1))
        v = lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return v;
}

std::string Luna::getFields(int tableIdx, const std::string &field,
        const std::string &dflt)
{
    std::string s(dflt);
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        s = lua_tostring(lua, -1);
    lua_pop(lua, 1);
    return s;
}

bool Luna::getFieldb(int tableIdx, const std::string &field, bool dflt)
{
    bool v = dflt;
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        v = lua_toboolean(lua, -1);
    lua_pop(lua, 1);
    return v;
}


int Luna::getFieldi(int tableIdx, const std::string &field, int dflt)
{
    int v = dflt;
    lua_getfield(lua, tableIdx, field.c_str());
    if (! lua_isnil(lua, -1))
        v = (int)lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return v;
}


int Luna::addRef()
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "xavionics");
    lua_pushvalue(lua, -2);
    int ref = luaL_ref(lua, -2);
    lua_pop(lua, 2);
    return ref;
}

void Luna::getRef(int ref)
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "xavionics");
    lua_rawgeti(lua, -1, ref);
    lua_remove(lua, -2);
}

void Luna::unRef(int ref)
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "xavionics");
    luaL_unref(lua, -1, ref);
}

void Luna::storeAvionics(Avionics *avionics)
{
    lua_pushlightuserdata(lua, avionics);
    lua_setfield(lua, LUA_REGISTRYINDEX, "avionics");
}

Avionics* xa::getAvionics(lua_State *lua)
{
    lua_getfield(lua, LUA_REGISTRYINDEX, "avionics");
    Avionics *v = (Avionics*)lua_touserdata(lua, -1);
    lua_pop(lua, 1);
    return v;
}

