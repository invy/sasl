#include "xplua.h"
#include <string>
#include <stdio.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "xpsdk.h"

using namespace xap;


/// reload scenery
static int reloadScenery(lua_State *L)
{
    XPLMReloadScenery();
    return 0;
}


// Lua wrapper for XPLMWorldToLocal function
// takes 3 double values (lat, lon, alt)
// returns 3 double values: x, y and z
static int worldToLocal(lua_State *L)
{
    double lat, lon, alt;
    double x, y, z;

    lat = lua_tonumber(L, 1);
    lon = lua_tonumber(L, 2);
    alt = lua_tonumber(L, 3);

    XPLMWorldToLocal(lat, lon, alt, &x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}

// Lua wrapper for XPLMLocalToWorld function
// takes 3 double values (x, y and z)
// returns 3 double values:lat, lon, alt
static int localToWorld(lua_State *L)
{
    double lat, lon, alt;
    double x, y, z;

    x = lua_tonumber(L, 1);
    y = lua_tonumber(L, 2);
    z = lua_tonumber(L, 3);

    XPLMLocalToWorld(x, y, z, &lat, &lon, &alt);

    lua_pushnumber(L, lat);
    lua_pushnumber(L, lon);
    lua_pushnumber(L, alt);

    return 3;
}


// NavAid API


// Lua wrapper for XPLMGetFirstNavAid function
static int getFirstNavAid(lua_State *L)
{
    lua_pushnumber(L, XPLMGetFirstNavAid());
    return 1;
}

// Lua wrapper for XPLMGetNextNavAid function
static int getNextNavAid(lua_State *L)
{
    lua_pushnumber(L, XPLMGetNextNavAid(lua_tonumber(L, 1)));
    return 1;
}

// Lua wrapper for XPLMGetFirstNavAidOfType function
static int findFirstNavAidOfType(lua_State *L)
{
    lua_pushnumber(L, XPLMFindFirstNavAidOfType(lua_tonumber(L, 1)));
    return 1;
}


// Lua wrapper for XPLMGetLastNavAidOfType function
static int findLastNavAidOfType(lua_State *L)
{
    lua_pushnumber(L, XPLMFindLastNavAidOfType(lua_tonumber(L, 1)));
    return 1;
}


// Lua wrapper for XPLMFindNavAid function
static int findNavAid(lua_State *L)
{
    const char *nameFragment = lua_isnil(L, 1) ? NULL : lua_tostring(L, 1);
    const char *idFragment = lua_isnil(L, 2) ? NULL : lua_tostring(L, 2);
    float lat = lua_tonumber(L, 3);
    float lon = lua_tonumber(L, 4);
    int freq = lua_tonumber(L, 5);
    int type = lua_tonumber(L, 6);
    
    lua_pushnumber(L, XPLMFindNavAid(nameFragment, idFragment, 
                lua_isnil(L, 3) ? NULL : &lat, lua_isnil(L, 4) ? NULL : &lon,
                lua_isnil(L, 5) ? NULL : &freq, type));
    return 1;
}


// Lua wrapper for XPLMGetNavAidInfo function
// gets reference to navaid
// return 9 results: type, lat, lon, alt, freq, heading, id, name, reg
static int getNavAidInfo(lua_State *L)
{
    XPLMNavType type=0;
    float lat=0, lon=0, alt=0;
    int freq=0;
    float heading=0;
    char id[256], name[256], reg[256];

    id[0] = name[0] = reg[0] = 0; // just in case
    XPLMGetNavAidInfo(lua_tonumber(L, 1), &type, &lat, &lon, &alt, &freq,
            &heading, id, name, reg);

    lua_pushnumber(L, type);
    lua_pushnumber(L, lat);
    lua_pushnumber(L, lon);
    lua_pushnumber(L, alt);
    lua_pushnumber(L, freq);
    lua_pushnumber(L, heading);
    lua_pushstring(L, id);
    lua_pushstring(L, name);
    lua_pushstring(L, reg);

    return 9;
}



// FMS API


// lua wrapper for XPLMCountFMSEntries
static int countFMSEntries(lua_State *L)
{
    lua_pushnumber(L, XPLMCountFMSEntries());
    return 1;
}


// lua wrapper for XPLMGetDisplayedFMSEntry
static int getDisplayedFMSEntry(lua_State *L)
{
    lua_pushnumber(L, XPLMGetDisplayedFMSEntry());
    return 1;
}

// lua wrapper for XPLMGetDestinationFMSEntry
static int getDestinationFMSEntry(lua_State *L)
{
    lua_pushnumber(L, XPLMGetDestinationFMSEntry());
    return 1;
}

// lua wrapper for XPLMSetDisplayedFMSEntry
static int setDisplayedFMSEntry(lua_State *L)
{
    XPLMSetDisplayedFMSEntry(lua_tonumber(L, 1));
    return 0;
}

// lua wrapper for XPLMSetDestinationFMSEntry
static int setDestinationFMSEntry(lua_State *L)
{
    XPLMSetDestinationFMSEntry(lua_tonumber(L, 1));
    return 0;
}

// lua wrapper for XPLMGetFMSEntryInfo
// returns 6 values: type, id, navAidRef, alt, lan and lon
static int getFMSEntryInfo(lua_State *L)
{
    XPLMNavType type=0;
    char id[256];
    XPLMNavRef navRef=-1;
    long alt=0;
    float lat=0, lon=0;

    id[0] = 0;
    XPLMGetFMSEntryInfo(lua_tonumber(L, 1), &type, id, &navRef, &alt,
            &lat, &lon);

    lua_pushnumber(L, type);
    lua_pushstring(L, id);
    lua_pushnumber(L, navRef);
    lua_pushnumber(L, alt);
    lua_pushnumber(L, lat);
    lua_pushnumber(L, lon);

    return 6;
}

// lua wrapper for XPLMSetFMSEntryInfo
static int setFMSEntryInfo(lua_State *L)
{
    XPLMSetFMSEntryInfo(lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3));
    return 0;
}

// lua wrapper for XPLMSetFMSEntryInfoLatLon
static int setFMSEntryLatLon(lua_State *L)
{
    XPLMSetFMSEntryLatLon(lua_tonumber(L, 1), lua_tonumber(L, 2), 
            lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}

// lua wrapper for XPLMClearFMSEntry
static int clearFMSEntry(lua_State *L)
{
    XPLMClearFMSEntry(lua_tonumber(L, 1));
    return 0;
}


// GPS API


// lua wrapper for XPLMGetGPSDestinationType
static int getGPSDestinationType(lua_State *L)
{
    lua_pushnumber(L, XPLMGetGPSDestinationType());
    return 1;
}

// lua wrapper for XPLMGetGPSDestination
static int getGPSDestination(lua_State *L)
{
    lua_pushnumber(L, XPLMGetGPSDestination());
    return 1;
}



// plugins api



// lua wrapper for XPLMGetMyID
static int getMyID(lua_State *L)
{
    lua_pushnumber(L, XPLMGetMyID());
    return 1;
}

// lua wrapper for XPLMCountPlugins
static int countPlugins(lua_State *L)
{
    lua_pushnumber(L, XPLMCountPlugins());
    return 1;
}

// lua wrapper for XPLMGetNthPlugin
static int getNthPlugin(lua_State *L)
{
    lua_pushnumber(L, XPLMGetNthPlugin(lua_tonumber(L, 1)));
    return 1;
}

// lua wrapper for XPLMFindPluginByPath
static int findPluginByPath(lua_State *L)
{
    lua_pushnumber(L, XPLMFindPluginByPath(lua_tostring(L, 1)));
    return 1;
}

// lua wrapper for XPLMFindPluginBySignature
static int findPluginBySignature(lua_State *L)
{
    lua_pushnumber(L, XPLMFindPluginBySignature(lua_tostring(L, 1)));
    return 1;
}

// lua wrapper for XPLMGetPluginInfo
// returns 4 strings: name, path, signature, description
static int getPluginInfo(lua_State *L)
{
    char name[256], path[256], sign[256], descr[256];
    name[0] = path[0] = sign[0] = descr[0] = 0;

    XPLMGetPluginInfo(lua_tonumber(L, 1), name, path, sign, descr);

    lua_pushstring(L, name);
    lua_pushstring(L, path);
    lua_pushstring(L, sign);
    lua_pushstring(L, descr);

    return 4;
}

// lua wrapper for XPLMIsPluginEnabled
static int isPluginEnabled(lua_State *L)
{
    lua_pushnumber(L, XPLMIsPluginEnabled(lua_tonumber(L, 1)));
    return 1;
}

// lua wrapper for XPLMEnablePlugin
static int enablePlugin(lua_State *L)
{
    lua_pushnumber(L, XPLMEnablePlugin(lua_tonumber(L, 1)));
    return 1;
}

// lua wrapper for XPLMDisablePlugin
static int disablePlugin(lua_State *L)
{
    XPLMDisablePlugin(lua_tonumber(L, 1));
    return 0;
}

// lua wrapper for XPLMReloadPlugins
static int reloadPlugins(lua_State *L)
{
    XPLMReloadPlugins();
    return 0;
}


// draw object API


// load object from file
static int loadObject(lua_State *L)
{
    if (lua_isnil(L, 1))
        return 0;

    XPLMObjectRef ref = XPLMLoadObject(lua_tostring(L, 1));
    if (! ref)
        return 0;
    lua_pushlightuserdata(L, ref);
    return 1;
}


// delete loaded object
static int unloadObject(lua_State *L)
{
    if (lua_islightuserdata(L, 1))
        return 0;
    XPLMUnloadObject((XPLMObjectRef)lua_touserdata(L, 1));
    return 0;
}


// draw loaded object
static int drawObject(lua_State *L)
{
    if (9 != lua_gettop(L))
        return 0;
    XPLMDrawInfo_t loc;
    XPLMObjectRef ref = (XPLMObjectRef)lua_touserdata(L, 1);
    if (! ref)
        return 0;

    loc.structSize = sizeof(loc);
    loc.x = lua_tonumber(L, 2);
    loc.y = lua_tonumber(L, 3);
    loc.z = lua_tonumber(L, 4);
    loc.pitch = lua_tonumber(L, 5);
    loc.heading = lua_tonumber(L, 6);
    loc.roll = lua_tonumber(L, 7);
    int lighting = lua_tonumber(L, 8);
    int earthRelative = lua_tonumber(L, 9);

    XPLMDrawObjects(ref, 1, &loc, lighting, earthRelative);

    return 0;
}


// register functions


// add integer const to Lua
static void registerConst(lua_State *L, const char *name, int value)
{
    lua_pushnumber(L, value);
    lua_setglobal(L, name);
}


void xap::exportLuaFunctions(lua_State *L)
{
    lua_register(L, "reloadScenery", reloadScenery);
    lua_register(L, "worldToLocal", worldToLocal);
    lua_register(L, "localToWorld", localToWorld);
 
    // navaid api

    registerConst(L, "NAV_UNKNOWN", 0);
    registerConst(L, "NAV_AIRPORT", 1);
    registerConst(L, "NAV_NDB", 2);
    registerConst(L, "NAV_VOR", 4);
    registerConst(L, "NAV_ILS", 8);
    registerConst(L, "NAV_LOCALIZER", 16);
    registerConst(L, "NAV_GLIDESLOPE", 32);
    registerConst(L, "NAV_OUTERMARKER", 64);
    registerConst(L, "NAV_MIDDLEMARKER", 128);
    registerConst(L, "NAV_INNERMARKER", 256);
    registerConst(L, "NAV_FIX", 512);
    registerConst(L, "NAV_DME", 1024);
    registerConst(L, "NAV_LATLON", 2048);
    registerConst(L, "NAV_NOT_FOUND", -1);

    lua_register(L, "getFirstNavAid", getFirstNavAid);
    lua_register(L, "getNextNavAid", getNextNavAid);
    lua_register(L, "findFirstNavAidOfType", findFirstNavAidOfType);
    lua_register(L, "findLastNavAidOfType", findLastNavAidOfType);
    lua_register(L, "findNavAid", findNavAid);
    lua_register(L, "getNavAidInfo", getNavAidInfo);

    // fms api
    lua_register(L, "countFMSEntries", countFMSEntries);
    lua_register(L, "getDisplayedFMSEntry", getDisplayedFMSEntry);
    lua_register(L, "getDestinationFMSEntry", getDestinationFMSEntry);
    lua_register(L, "setDisplayedFMSEntry", setDisplayedFMSEntry);
    lua_register(L, "setDestinationFMSEntry", setDestinationFMSEntry);
    lua_register(L, "getFMSEntryInfo", getFMSEntryInfo);
    lua_register(L, "setFMSEntryInfo", setFMSEntryInfo);
    lua_register(L, "setFMSEntryLatLon", setFMSEntryLatLon);
    lua_register(L, "clearFMSEntry", clearFMSEntry);

    // gps api
    lua_register(L, "getGPSDestinationType", getGPSDestinationType);
    lua_register(L, "getGPSDestination", getGPSDestination);

    // plugins api
    lua_register(L, "getMyID", getMyID);
    lua_register(L, "countPlugins", countPlugins);
    lua_register(L, "getNthPlugin", getNthPlugin);
    lua_register(L, "findPluginByPath", findPluginByPath);
    lua_register(L, "findPluginBySignature", findPluginBySignature);
    lua_register(L, "getPluginInfo", getPluginInfo);
    lua_register(L, "isPluginEnabled", isPluginEnabled);
    lua_register(L, "enablePlugin", enablePlugin);
    lua_register(L, "disablePlugin", disablePlugin);
    lua_register(L, "reloadPlugins", reloadPlugins);

    // objects api
    lua_register(L, "loadObject", loadObject);
    lua_register(L, "unloadObject", unloadObject);
    lua_register(L, "drawObject", drawObject);
}


