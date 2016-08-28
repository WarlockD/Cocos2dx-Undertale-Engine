#pragma once
#include <cassert>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <atomic>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <type_traits>
#include <chrono>

#ifndef CONSOLE_SCREEN_BUFFER_INFO
struct _CONSOLE_SCREEN_BUFFER_INFOEX;
#endif

#ifdef max
#undef max
//#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifdef min
#undef min
//#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

namespace console {
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
	inline constexpr typename std::enable_if< std::is_enum<E>::value,bool>::type test_flag(E l, E r) {
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
	// use % to test if the flag is inside the other
#define ENUM_OPERATIONS(T) \
	inline constexpr T operator|(T l, T r) { return to_enum<T>(to_underlying(l) | to_underlying(r)); } \
	inline constexpr T operator&(T l, T r) { return to_enum<T>(to_underlying(l) & to_underlying(r));  } \
	inline T& operator|=(T& l, T r) { l = l | r; return l; } \
	inline T& operator&=(T& l, T r) { l = l & r; return l; } \
	inline constexpr bool operator%(T l, T r) { return (to_underlying(l) & to_underlying(r)) != 0; } 
	
	/// math stuff
	template<class T> inline const T& max(const T& a, const T& b) { return (a < b) ? b : a; }
	//	template<class T, class Compare> inline const T& max(const T& a, const T& b, Compare comp) { return (comp(a, b)) ? b : a; }
	template<class T> inline const T& min(const T& a, const T& b) { return (b < a) ? b : a; }
	template<class T, class Compare>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
	{
		return assert(!comp(hi, lo)),
			comp(v, lo) ? lo : comp(hi, v) ? hi : v;
	}
	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi)
	{
		return clamp(v, lo, hi, std::less<>());
	}
	template<typename T> constexpr T pow_const(T x, unsigned int y) { return y > 1 ? x * pow_const(x, y - 1) : x; }
	template<typename T> inline T pow(T x, unsigned int y) { T r = x; while (y-- > 1) r *= x; return r; }
	template<typename T> constexpr T abs_const(T v) { return v >= 0 ? v : -v; }
	template<typename T> inline T abs(T v) { return std::abs(v); }

	// Enums for console
	enum class Mode : uint16_t
	{
		ProcessedInput = 0x1,
		LineInput = 0x2,
		EchoInput =0x4,
		WindowInput = 0x8,
		MouseInput = 0x10,
		InsertMode = 0x20,
		QuickEditMode = 0x40,
		ExtendedFlags = 0x80,
		AutoPosition = 0x100,
		HideCtrlC = ProcessedInput, 
		Echo = EchoInput | InsertMode,
		EnableMouseSelection = QuickEditMode | ExtendedFlags,
		
		Override = 0x200, //Override = ~InsertMode | ExtendedFlags
	};
	ENUM_OPERATIONS(Mode);

	enum class  Color : uint8_t {
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

	enum class EventType : uint16_t {
		Key = 0x0001, // Event contains key event record
		Mouse = 0x0002, // Event contains mouse event record
		Resize = 0x0004, // Event contains window change event record
		Menu = 0x0008, // Event contains menu event record
		Focus = 0x0010, // event contains focus change
	};
	ENUM_OPERATIONS(EventType);

	enum class KeyState : unsigned long {
		RightAltDown = 0x0001, // the right alt key is pressed.
		LeftAltDown = 0x0002, // the left alt key is pressed.
		RightCtrlDown = 0x0004,// the right ctrl key is pressed.
		LeftCtrlDown = 0x0008, // the left ctrl key is pressed.
		ShiftDown = 0x0010, // the shift key is pressed.
		NumberLockOn = 0x0020, // the numlock light is on.
		ScrollLockOn = 0x0040, // the scrolllock light is on.
		CapsLockOn = 0x0080, // the capslock light is on.
		EnhancedKey = 0x0100, // the key is enhanced.
	};
	ENUM_OPERATIONS(KeyState);

	enum class MouseButtonState : unsigned long {
		FromLeftButton1Pressed = 0x0001, // the right alt key is pressed.
		RightMostButtonPressed = 0x0002, // the left alt key is pressed.
		FromLeftButton2Pressed = 0x0004,// the right ctrl key is pressed.
		FromLeftButton3Pressed = 0x0008, // the left ctrl key is pressed.
		FromLeftButton4Pressed = 0x0010, // the shift key is pressed.
	};
	ENUM_OPERATIONS(MouseButtonState);

	enum class MouseState : unsigned long {
		Moved = 0x0001,
		DoubleClicked = 0x0002,
		Wheel = 0x0004,
		HWheel = 0x0008,
	};
	ENUM_OPERATIONS(MouseState);


	struct CharInfo {
		static const CharInfo Blank; // default, space that is on a white forground with a black bachkground
		union { struct { union { wchar_t wch; char ch; }; uint16_t attrib; }; uint32_t int_value; };
		CharInfo() : CharInfo(Blank) {}
		constexpr CharInfo(char ch, Color fg = Color::White, Color bg = Color::Black) : ch(' '), attrib(static_cast<uint8_t>(fg) | (static_cast<uint8_t>(bg) << 4)) {}
		//  with black background and white forground
		Color fg() const { return static_cast<Color>(attrib & 0xF); }
		Color bg() const { return static_cast<Color>((attrib >> 4) & 0xF); }
		void fg(Color c) { attrib = (attrib & 0xF0) | static_cast<uint8_t>(c); }
		void bg(Color c) { attrib = (attrib & 0x0F) | (static_cast<uint8_t>(c) << 4); }
	};
	inline bool operator==(const CharInfo &l, const CharInfo &r) { return l.int_value == r.int_value; }
	inline bool operator!=(const CharInfo &l, const CharInfo &r) { return l.int_value != r.int_value; }
	inline bool operator==(const CharInfo &l, char r) { return l.ch == r; }
	inline bool operator!=(const CharInfo &l, char r) { return l.ch != r; }
	inline bool operator==(const CharInfo &l, wchar_t r) { return l.wch == r; }
	inline bool operator!=(const CharInfo &l, wchar_t r) { return l.wch != r; }


	struct Point {
		union { struct { int16_t x; int16_t y; }; int16_t ptr[2]; uint32_t int_value; };
		static constexpr size_t dimensions = 2;
		Point() : x(0), y(0) {}
		Point& set(int16_t X, int16_t Y) {x = X; y = Y;return *this;} 
		explicit Point(int16_t X, int16_t Y) : x(X), y(Y) {}


		inline Point operator-() const { return Point(-x, -y); }
		inline Point& operator/=(const int16_t r) { x /= r; y /= r;  return *this; }
		inline Point& operator*=(const int16_t r) { x *= r; y *= r;  return *this; }
		inline Point& operator/=(const Point &r) { x /= r.x; y /= r.y;  return *this; }
		inline Point& operator*=(const Point &r) { x *= r.x; y *= r.y;  return *this; }
		inline Point& operator+=(const Point &r) { x -= r.x; y -= r.y;  return *this; }
		inline Point& operator-=(const Point &r) { x += r.x; y += r.y;  return *this; }
	};
	inline bool operator==(const Point &l, const Point &r) { return l.int_value == r.int_value; }
	inline bool operator!=(const Point &l, const Point &r) { return l.int_value != r.int_value; }
	inline Point operator+(const Point &l, const Point &r) { return Point(l.x + r.x, l.y + r.y); }
	inline Point operator-(const Point &l, const Point &r) { return Point(l.x - r.x, l.y - r.y); }

	struct Rect {
		union { struct { int16_t top; int16_t left; int16_t right; int16_t bottom; }; int16_t ptr[4]; uint64_t int_value; };
		int16_t width() const { return right - left; }
		int16_t height() const { return bottom - top; }
		void width(int16_t width) { right += width - right - left; }
		void height(int16_t height) { right += height - bottom - top; }
		Rect() : int_value(0) {}
		explicit Rect(const Point& top_left, const Point& bottom_right) : top(top_left.y), left(top_left.x), bottom(bottom_right.y), right(bottom_right.x) {}
		explicit Rect(int16_t top, int16_t left, int16_t right, int16_t bottom) : top(top), left(left), bottom(bottom), right(right) {}
		Rect& set(int16_t top, int16_t left, int16_t right, int16_t bottom) { return *this = Rect(top, left, right, bottom); }
		inline Rect& operator*=(const int16_t &r) { top *= r; bottom *= r; right *= r; left *= r; return *this; }
		inline Rect& operator/=(const int16_t &r) { top /= r; bottom /= r; right /= r; left /= r; return *this; }
		inline Rect& operator+=(const Rect &r) { top += r.top; bottom += r.bottom; right += r.right; left += r.left; return *this; }
		inline Rect& operator-=(const Rect &r) { top -= r.top; bottom -= r.bottom; right -= r.right; left -= r.left; return *this; }
		inline Rect& operator+=(const Point &r) { top += r.y; bottom += r.y; right += r.x; left += r.x; return *this; }
		inline Rect& operator-=(const Point &r) { top -= r.y; bottom -= r.y; right -= r.x; left -= r.x; return *this; }
		inline Rect operator-() const { return Rect(-top, -left, -bottom, -right); }
		bool contains(int16_t x, int16_t y) const { return (x >= left) && (y <= right) && (y >= top) && (y <= bottom); }
		bool contains(const Point& p) const { return contains(p.x, p.y); }
		bool intersects(const Rect& rect, Rect& overlap) const {
			overlap = Rect(console::max(left, rect.left), console::max(top, rect.top),
				console::min(right, rect.right), console::min(bottom, rect.bottom));
			// If overlapping rect is valid, then there is intersection
			return ((overlap.left < overlap.right) && (overlap.top < overlap.bottom));
		}
		bool intersects(const Rect& rect) const { Rect overlap; return intersects(rect, overlap); }
	};
	inline bool operator==(const Rect &l, const Rect &r) { return l.top == r.top && l.left == r.left && l.right == r.right&& l.bottom == r.bottom; }
	inline bool operator!=(const Rect &l, const Rect &r) { return !(l == r); }
	inline Rect operator*(const Rect &l, const int16_t &r) { return Rect(l.top * r, l.left * r, l.right * r, l.bottom * r); }
	inline Rect operator/(const Rect &l, const int16_t &r) { return Rect(l.top / r, l.left / r, l.right / r, l.bottom / r); }
	inline Rect operator+(const Rect &l, const Rect &r) { return Rect(l.top + r.top, l.left + r.left, l.right + r.right, l.bottom + r.bottom); }
	inline Rect operator-(const Rect &l, const Rect &r) { return Rect(l.top - r.top, l.left - r.left, l.right - r.right, l.bottom - r.bottom); }
	inline Rect operator+(const Rect &l, const Point &r) { return Rect(l.top + r.y, l.left + r.x, l.right + r.x, l.bottom + r.y); }
	inline Rect operator-(const Rect &l, const Point &r) { return Rect(l.top - r.y, l.left - r.x, l.right - r.x, l.bottom - r.y); }

	struct ScreenInfo {
		Point size;
		Point cursor;
		uint16_t attrib;
		Rect window;
		Point max_window_size;
	};


	struct KeyEvent {
		int keyDown;
		uint16_t RepeatCount;
		uint16_t VirtualKeyCode;
		uint16_t VirtualScanCode;
		union {
			wchar_t UnicodeChar;
			char   AsciiChar;
		} Char;
		KeyState dwControlKeyState;
	};
	struct MouseEvent {
		Point Position;
		MouseButtonState ButtonState;
		KeyState ControlKeyState;
		MouseState EventFlags;
	};
	struct ResizeEvent {
		Point Size;
	};
	struct MenuEvent {
		uint32_t CommandId;
	};
	struct FocusEvent {
		int SetFocus;
	};
	struct InputEvent {
		EventType EventType;
		union {
			KeyEvent KeyEvent;
			MouseEvent MouseEvent;
			ResizeEvent WindowBufferSizeEvent;
			MenuEvent MenuEvent;
			FocusEvent FocusEvent;
		} Event;
	};

	struct TerminalSettings {
		bool scroll_on_linefeed;
		bool return_on_linefeed; // do a \r after a \n
		CharInfo default;
		TerminalSettings() : scroll_on_linefeed(true), return_on_linefeed(true) ,default(CharInfo::Blank) {}
	};
	class Image
	{
		Point _size;
		std::vector<CharInfo> _chars;
		CharInfo _default;
		typedef std::vector<CharInfo>::iterator iterator;
		typedef std::vector<CharInfo>::const_iterator const_iterator;
	public:
		Image() : _size(0, 0), _default(' ', Color::White, Color::Black) {}
		Image(int16_t width, int16_t height, const char fill = ' ', Color fg = Color::White, Color bg = Color::Black) : _size(width, height),
			_default(fill, fg, bg), _chars(width*height, _default) {}
		Image(int16_t width, int16_t height, Color fg = Color::White, Color bg = Color::Black) : _size(width, height),
			_default(' ', fg, bg), _chars(width*height, _default) {}
		int16_t width() const { return _size.x; }
		int16_t height() const { return _size.y; }
		CharInfo& at(size_t x, size_t y) { return _chars[x + y * _size.x]; }
		const CharInfo& at(size_t x, size_t y) const { return _chars[x + y * _size.x]; }
		iterator begin() { return _chars.begin(); }
		iterator end() { return _chars.end(); }
		const_iterator begin() const { return _chars.begin(); }
		const_iterator end() const { return _chars.end(); }
		iterator lbegin(int16_t y) { return _chars.begin() + (y * _size.x); }
		iterator lend(int16_t y) { return _chars.end() + (y * _size.x) + _size.x; }
		const_iterator lbegin(int16_t y) const { return _chars.begin() + (y * _size.x); }
		const_iterator lend(int16_t y) const { return _chars.end() + (y * _size.x) + _size.x;; }
		CharInfo* data() { return _chars.data(); }
		const CharInfo* data() const { return _chars.data(); }
		size_t size() const { return _chars.size(); }
		CharInfo& operator[](size_t i) { return _chars[i]; }
		const CharInfo& operator[](size_t i) const { return _chars[i]; }
	};

	class output_context
	{
		ScreenInfo _info;
		Mode _mode;
	public:
		const ScreenInfo& info() const { return _info; }
		const Mode& mode() const { return _mode; }
		void restore();
		void save();
		output_context() { save(); }
		~output_context() { restore(); }
	};

	void gotoxy(int x, int y);
	inline void gotoxy(const Point& p) { gotoxy(p.x, p.y); }
	void gotox(int x);
	void gotoy(int y);
	Point cursor();
	
	void cls();
	void mode(Mode m);
	void background(Color c);
	void foreground(Color c);

	void init();


	std::ostream& vt100(); // stream for vt100 emulation on console, only simple escape codes are supported however
};

namespace con {
	// stream manipulators
	struct cls {
		explicit cls() {}
	};
	
	struct background {
		console::Color c;
		explicit background(console::Color c) : c(c) {}
	};
	
	struct foreground {
		console::Color c;
		explicit foreground(console::Color c) : c(c) {}
	};
	
	struct mode {
		console::Mode m;
		explicit mode(console::Mode m) : m(m) {}
	};
	
	struct gotoxy {
		console::Point p;
		explicit gotoxy(int x, int y) : p(x, y) {}
		explicit gotoxy(const console::Point& p) : p(p) {}
	};
	
	struct gotox {
		int x;
		explicit gotox(int x) : x(x) {}
	};
	
	struct gotoy {
		int y;
		explicit gotoy(int y) : y(y) {}
	};
	inline std::ostream& operator<<(std::ostream& os, const cls& l) { console::cls(); return os; }
	inline std::ostream& operator<<(std::ostream& os, const background& l) { console::background(l.c); return os; }
	inline std::ostream& operator<<(std::ostream& os, const foreground& l) { console::foreground(l.c); return os; }
	inline std::ostream& operator<<(std::ostream& os, const mode& l) { console::mode(l.m); return os; }
	inline std::ostream& operator<<(std::ostream& os, const gotoxy& l) { console::gotoxy(l.p); return os; }
	inline std::ostream& operator<<(std::ostream& os, const gotox& l) { console::gotox(l.x); return os; }
	inline std::ostream& operator<<(std::ostream& os, const gotoy& l) { console::gotoy(l.y); return os; }
	

};


namespace logging {
	void init_cerr();
	void init_cout();
	bool init();
	void error(const std::string& message);
};