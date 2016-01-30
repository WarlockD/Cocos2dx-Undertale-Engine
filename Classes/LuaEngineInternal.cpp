#include "LuaEngine.h"
#include <cstdarg>

extern "C" {    // another way
#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
};


static lua_State* _luaState = nullptr;
/*
Above is the structure for strings in lua.  So we need to go back size_t for the len and unsigned int for the hash
This is DANGROUS but it saves time and space
The string is a buffer at the end of the TString var.
*/
//typedef struct istring_internal TString;

inline static const struct TString* convertFromTString(const char* s) {
	return reinterpret_cast<const  struct TString*>(s - sizeof(struct TString));
}