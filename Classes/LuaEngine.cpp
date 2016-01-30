#include "cocos2d.h"

#include "UndertaleResources.h"
#include <unordered_set>
#include <fstream>
#include "LuaEngine.h"

extern "C" {    // another way
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
	#include "lstring.h"
};

USING_NS_CC;


/*
Above is the structure for strings in lua.  So we need to go back size_t for the len and unsigned int for the hash
This is DANGROUS but it saves time and space
The string is a buffer at the end of the TString var.
*/
//struct istring_internal { UTString utsr; }; // we don't really make this, we just dynamic cast it to UTString


struct hash_TString {
	inline size_t operator()(const TString *l) const{ return l->hash; }
};

class internal_luaEngine {
	static internal_luaEngine* s_maker;
	static const char* emptyString;
	static lua_State* L ;
	internal_luaEngine() {
		L = luaL_newstate();
		emptyString = getStringInterned("");
	}
public:
	inline const char* getEmptyString() const {
		return emptyString;
	}
	
	// some internal lua trickery, works for 5.3 but note sure for
	// others
	inline const TString* getTStringInterned(const char*s) const { // inlined as there are only a few times we use this
		luaC_checkGC(L); // check the GC!
		// side note, you WANT to use luaS_new.  Lua 5.3 caches the pointers for const char* so if its a a literal, it will look up much faster
		// I thought of doing the same here with a map, but why do it twice?
		TString*  ts = luaS_new(L, s);
		GCObject *o = obj2gco(ts);
		if (G(L)->allgc == o) // if created it
			luaC_fix(L, o); // fix it so it can't be collected
		return ts;
	}
	inline const char* getStringInterned(const char* s) const { return getstr(getTStringInterned(s)); }
public: // get instance and other public static functions
	inline static internal_luaEngine* getInstance() {
		if (!s_maker) s_maker = new internal_luaEngine();
		return s_maker;
	}
};

internal_luaEngine* internal_luaEngine::s_maker = nullptr;
const char* internal_luaEngine::emptyString = nullptr;
lua_State* internal_luaEngine::L = nullptr;

istring::istring() : _internal(internal_luaEngine::getInstance()->getEmptyString()) { }
istring::istring(const char * str) : _internal(internal_luaEngine::getInstance()->getStringInterned(str)) { }
istring::istring(const std::string & str) : istring(str.c_str()) {}
size_t istring::length() const { return tsslen((TString*)(_internal - sizeof(TString))); }
size_t istring::hash() const { return ((TString*)(_internal - sizeof(TString)))->hash; }


