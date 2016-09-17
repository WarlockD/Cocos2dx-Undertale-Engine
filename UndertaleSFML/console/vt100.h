#include <type_traits>
#include <iostream>




namespace vt100 {
	void init();
	void print(wchar_t ch);
	inline void print(char ch) { print((wchar_t)ch); }
	void print(const char* message);
	
};