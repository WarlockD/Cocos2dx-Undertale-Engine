#pragma once
#include <ctype.h>
#include <vector>
#include <type_traits>

namespace curses {
	namespace priv {
		// helpers for enum operations
		// http://stackoverflow.com/questions/8357240/how-to-automatically-convert-strongly-typed-enum-into-int
		template <typename E>
		inline constexpr typename std::underlying_type<E>::type to_underlying(E e) {
			return static_cast<typename std::underlying_type<E>::type>(e);
		}
		template< typename E, typename T>
		inline constexpr typename std::enable_if< std::is_enum<E>::value && std::is_integral<T>::value, E >::type
			to_enum(T value) noexcept { return static_cast<E>(value); }

		template<typename E>
		inline constexpr typename std::enable_if< std::is_enum<E>::value, bool>::type test_flag(E l, E r) {
			return (to_underlying(l)  & to_underlying(r)) != 0;
		}
		template<typename E>
		inline constexpr typename std::enable_if< std::is_enum<E>::value, E&>::type set_flag(E& l, E r) {
			l = to_enum<E>(to_underlying(l) | to_underlying(r));
			return l;
		}
		template<typename E>
		inline constexpr typename std::enable_if< std::is_enum<E>::value, E&>::type clear_flag(E& l, E r) {
			l = to_enum<E>(to_underlying(l) & ~to_underlying(r));
			return l;
		}
	};
//	typedef unsigned long chtype;  /* 16-bit attr + 16-bit char */
								   // use % to test if the flag is inside the other
#define ENUM_OPERATIONS(T) \
	inline constexpr T operator|(T l, T r) { return priv::to_enum<T>(priv::to_underlying(l) | priv::to_underlying(r)); } \
	inline constexpr T operator&(T l, T r) { return priv::to_enum<T>(priv::to_underlying(l) & priv::to_underlying(r));  } \
	inline T& operator|=(T& l, T r) { l = l | r; return l; } \
	inline T& operator&=(T& l, T r) { l = l & r; return l; } \
	inline constexpr bool operator%(T l, T r) { return (priv::to_underlying(l) & priv::to_underlying(r)) != 0; } 

	enum class  Color : unsigned char {
		Black = 0,
		DarkBlue = 1,
		DarkGreen = 2,
		DarkRed = 4,
		Intensity = 8,
		DarkCyan = DarkGreen | DarkBlue,
		DarkMagenta = DarkRed | DarkBlue,
		DarkYellow = DarkRed | DarkGreen,
		DarkGray = DarkRed | DarkGreen | DarkBlue,
		Gray = Intensity,
		Green = Intensity | DarkGreen,
		Blue = Intensity | DarkBlue,
		Red = Intensity | DarkRed,
		Magenta = Intensity | DarkRed | DarkBlue,
		Yellow = Intensity | DarkRed | DarkGreen,
		White = Intensity | DarkRed | DarkGreen | DarkBlue,
	};
	ENUM_OPERATIONS(Color);
	enum class Attributes : unsigned char {
		Normal = 0,
		Reverse = 0x02,
		Blink = 0x04,
	};
	ENUM_OPERATIONS(Attributes);

	// I hate poluting the macro world
#undef ENUM_OPERATIONS
	struct char_info {
		unsigned short ch;
		unsigned short attrib;
		void forground(Color f) { attrib |= (attrib & 0xFFF0) | (static_cast<unsigned char>(f) & 0xF); }
		void background(Color b) { attrib |= (attrib & 0xFF0F) | ((static_cast<unsigned char>(b) & 0xF) << 4); }
		void attributes(Attributes a) { attrib |= (attrib & 0x00FF) | ((static_cast<unsigned char>(a)) << 8); }
		Color forground() const { return static_cast<Color>(attrib & 0x0F); }
		Color background() const { return static_cast<Color>((attrib>>4) & 0x0F); }
		Attributes attributes() const { return static_cast<Attributes>((attrib >> 8) & 0xFF); }
	};
	typedef unsigned int chtype;
	union uchtype {
		char_info info;
		chtype type;
	};

	struct Window       /* definition of a window */
	{
		int   _cury;          /* current pseudo-cursor */
		int   _curx;
		int   _maxy;          /* max window coordinates */
		int   _maxx;
		int   _begy;          /* origin on screen */
		int   _begx;
		int   _flags;         /* window properties */
		chtype _attrs;        /* standard attributes and colors */
		chtype _bkgd;         /* background, normally blank */
		bool  _clear;         /* causes clear at next refresh */
		bool  _leaveit;       /* leaves cursor where it is */
		bool  _scroll;        /* allows window scrolling */
		bool  _nodelay;       /* input character wait flag */
		bool  _immed;         /* immediate update flag */
		bool  _sync;          /* synchronise window ancestors */
		bool  _use_keypad;    /* flags keypad key mode active */
		chtype **_y;          /* pointer to line pointer array */
		int   *_firstch;      /* first changed character in line */
		int   *_lastch;       /* last changed character in line */
		int   _tmarg;         /* top of scrolling region */
		int   _bmarg;         /* bottom of scrolling region */
		int   _delayms;       /* milliseconds of delay for getch() */
		int   _parx, _pary;   /* coords relative to parent (0,0) */
		Window *_parent;	  /* subwin's pointer to parent win */

	};
	
	extern  int          LINES;        /* terminal height */
	extern  int          COLS;         /* terminal width */
	extern  Window       *stdscr;      /* the default screen window */
	extern  Window       *curscr;      /* the current screen image */
									   //extern  SCREEN       *SP;          /* curses variables */
									   //extern  MOUSE_STATUS Mouse_status;
	extern  int          COLORS;
	extern  int          COLOR_PAIRS;
	extern  int          TABSIZE;
	//extern  chtype       acs_map[];    /* alternate character set map */
//	extern  char         ttytype[];    /* terminal name/description */

};