#pragma once
#include "cocos2d.h"


// Simple,stupid string class that uses cocos2d lua engine to intern strings
// the const char* returned by this are always valid and the same thoughout 
// Also, side note, while we have TSTring here from the lua structure
// its ALWAYS going to be a short string.  We will throw something if you try to put something
// longer than 
class istring  {
	const struct TString* _internal;
public:
	typedef const char* const_iterator;
public: // constructors and moves
	istring();
	istring(const char* str);
	istring(const std::string& str);

	const char* c_str() const;
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

	inline bool isEmpty() const { return c_str()[0] == 0; } // hackery, but true
};

// hash function for istring
namespace std {
	template <> struct hash<istring> { std::size_t operator()(const istring& k) const { return k.hash(); } };
}
