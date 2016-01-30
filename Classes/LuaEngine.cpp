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
	static const TString* emptyString;
	static lua_State* L ;
	internal_luaEngine() {
		L = luaL_newstate();
		emptyString = getInternedString("");
	}
public:
	inline const TString* getEmptyString() const {
		return emptyString;
	}
	
	// some internal lua trickery, works for 5.3 but note sure for
	// others
	inline const TString* getInternedString(const char*s) const { // inlined as there are only a few times we use this
		luaC_checkGC(L); // not sure if we need to run this but just to be sure
		TString*  ts = luaS_new(L, s);
		GCObject *o = obj2gco(ts);
		if (G(L)->allgc == o) // if created it
			luaC_fix(L, o); // fix it so it can't be collected
		return ts;
	}
public: // get instance and other public static functions
	inline static internal_luaEngine* getInstance() {
		if (!s_maker) s_maker = new internal_luaEngine();
		return s_maker;
	}
};
internal_luaEngine* internal_luaEngine::s_maker = nullptr;
const TString* internal_luaEngine::emptyString = nullptr;
lua_State* internal_luaEngine::L = nullptr;

istring::istring() : _internal(internal_luaEngine::getInstance()->getEmptyString()) { }
istring::istring(const char * str) : _internal(internal_luaEngine::getInstance()->getInternedString(str)) { }
istring::istring(const std::string & str) : istring(str.c_str()) {}

const char * istring::c_str() const { return getstr(_internal); }
size_t istring::length() const { return tsslen(_internal); }
size_t istring::hash() const { return _internal->hash; }


