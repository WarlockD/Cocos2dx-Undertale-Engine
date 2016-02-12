#pragma once
#include "cocos2d.h"
#include "lua.hpp"

// Originaly we kept TString from lua in here, till I realized that you hardly use it AT ALL
// outside of this class unless, by chance, your using the "hash" or "length" function.
// This is mainly for equality and comapring small strings anyway
class istring  {
	//const struct TString* _internal;
	const char* _internal;
public:
	typedef const char* const_iterator;
public: 
	// rule of 5
	istring();
	//istring(const istring& s) : _internal(s._internal) {}
	//istring(istring&& s) : _internal(s._internal) {  } // side note, we don't want _internal ever to be null
	//istring& operator=(istring&& s) { *this = istring(s); return *this; }
	//istring& operator=(const istring& s) { *this = istring(s); return *this; }

	istring(const char* str);
	istring(const std::string& str);
	istring& operator=(const char*s) { *this = istring(s); return *this; }
	istring& operator=(const std::string& s) { *this = istring(s); return *this; }

	inline const char* c_str() const { return _internal; }


	size_t length() const;
	size_t hash() const;

	inline const_iterator begin() const noexcept { return c_str(); }
	inline const_iterator end() const noexcept { return c_str()+length(); }

	// use std::strings as buffers if you just want to check equalty but don't want to make a 
	// new istring
	inline bool operator==(const std::string& str) const { return c_str() == str; }
	inline bool operator!=(const std::string& str) const { return c_str() != str; }

	inline bool operator==(const istring& str) const { return _internal == str._internal; }
	inline bool operator!=(const istring& str) const { return _internal != str._internal; }
	inline bool operator==(const char* str) const { return  *this == istring(str); }
	inline bool operator!=(const char* str) const { return  *this != istring(str); }
	// for sets and standard map
	inline bool operator<(const istring& str) const { return  hash() < str.hash(); }

	bool isEmpty() const; // hackery, but true since _internal is never null, or shouldn't be?
};

// hash function for istring
namespace std {
	template <> struct hash<istring> { std::size_t operator()(const istring& k) const { return k.hash(); } };
}
//inline bool operator==(const istring& l, const istring& r)  { return l.c_str() == r.c_str(); }
//inline bool operator!=(const istring& l, const istring& r) { return l.c_str() != r.c_str(); }



// loving constexpr
namespace LuaEngineMetaTableNames {
	template<typename T> constexpr const char* metaTableName() { throw std::runtime_error("No metatable is defined!"); }
	template<> constexpr const char* metaTableName<cocos2d::Vec2>() { return "Vec2MT"; }
}

class LuaEngine {
public:
	template<typename T> inline static  T* newUserData(lua_State*L, const char* metaname) {
		T* obj = static_cast<T*>(lua_newuserdata(L, sizeof(T)));
		luaL_getmetatable(L, metaname);
		lua_setmetatable(L, -2);
		return obj;
	}
	template<typename T>  inline static  T* newUserData(lua_State*L) {
		return newUserData<T>(L, LuaEngineMetaTableNames::metaTableName<T>());
	}

	template<typename T>  inline static  T** newUserData(lua_State*L, T* ptr, const char* metaname) {
		T** obj = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
		luaL_getmetatable(L, metaname);
		lua_setmetatable(L, -2);
		*obj = ptr;
		return obj;
	}
	template<typename T>  inline  static T** newUserData(lua_State*L, T* ptr) {
		return newUserData<T>(L, ptr, LuaEngineMetaTableNames::metaTableName<T>());
	}
	
	static lua_State* getLuaState();
	static void  setLuaScene(cocos2d::Node* scene);// The scene that all your lua nodes go into
	static cocos2d::Node*  getLuaScene();// The scene that all your lua nodes go into
	static bool RunGlobalUpdate(const char* global_name, float dt);
	static bool DoFile(const char* filename);
};


