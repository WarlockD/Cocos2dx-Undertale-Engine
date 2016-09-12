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
#include <list>

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

	typedef unsigned int chtype;
	struct CharInfo {
		static const CharInfo Blank; // default, space that is on a white forground with a black bachkground
		union { struct { union { wchar_t wch; char ch; }; uint16_t attrib; }; chtype value; };
		CharInfo() : CharInfo(Blank) {}
		constexpr CharInfo(char ch, Color fg = Color::White, Color bg = Color::Black) : ch(' '), attrib(static_cast<uint8_t>(fg) | (static_cast<uint8_t>(bg) << 4)) {}
		//  with black background and white forground
		Color fg() const { return static_cast<Color>(attrib & 0xF); }
		Color bg() const { return static_cast<Color>((attrib >> 4) & 0xF); }
		void fg(Color c) { attrib = (attrib & 0xF0) | static_cast<uint8_t>(c); }
		void bg(Color c) { attrib = (attrib & 0x0F) | (static_cast<uint8_t>(c) << 4); }
	};
	inline bool operator==(const CharInfo &l, const CharInfo &r) { return l.value == r.value; }
	inline bool operator!=(const CharInfo &l, const CharInfo &r) { return l.value != r.value; }
	inline bool operator==(const CharInfo &l, char r) { return l.ch == r; }
	inline bool operator!=(const CharInfo &l, char r) { return l.ch != r; }
	inline bool operator==(const CharInfo &l, wchar_t r) { return l.wch == r; }
	inline bool operator!=(const CharInfo &l, wchar_t r) { return l.wch != r; }


	struct Point {
		int16_t x; 
		int16_t y;
		static constexpr size_t dimensions = 2;
		static const Point Up;
		static const Point Down;
		static const Point Left;
		static const Point Right;
		Point() : x(0), y(0) {}
		Point& set(int16_t X, int16_t Y) {x = X; y = Y;return *this;} 
		Point(int16_t X, int16_t Y) : x(X), y(Y) {}
		uint32_t hash() const { return static_cast<uint32_t>(y) << 16 | static_cast<uint32_t>(x); }
		inline Point operator-() const { return Point(-x, -y); }
		inline Point& operator/=(const int16_t r) { x /= r; y /= r;  return *this; }
		inline Point& operator*=(const int16_t r) { x *= r; y *= r;  return *this; }
		inline Point& operator/=(const Point &r) { x /= r.x; y /= r.y;  return *this; }
		inline Point& operator*=(const Point &r) { x *= r.x; y *= r.y;  return *this; }
		inline Point& operator+=(const Point &r) { x -= r.x; y -= r.y;  return *this; }
		inline Point& operator-=(const Point &r) { x += r.x; y += r.y;  return *this; }
	};
	inline bool operator==(const Point &l, const Point &r) { return l.hash() == r.hash(); }
	inline bool operator!=(const Point &l, const Point &r) { return l.hash() != r.hash(); }
	inline Point operator+(const Point &l, const Point &r) { return Point(l.x + r.x, l.y + r.y); }
	inline Point operator-(const Point &l, const Point &r) { return Point(l.x - r.x, l.y - r.y); }
	inline Point operator*(const Point &l, const Point &r) { return Point(l.x * r.x, l.y * r.y); }
	inline Point operator*(const Point &l, int16_t r) { return Point(l.x * r, l.y * r); }
	inline Point operator*(int16_t r, const Point &l) { return Point(l.x * r, l.y * r); }

	struct Rect {
		int16_t top; 
		int16_t left; 
		int16_t right; 
		int16_t bottom;
		int16_t width() const { return right - left; }
		int16_t height() const { return bottom - top; }
		uint32_t hash() const { return (static_cast<uint32_t>(top) << 16 | static_cast<uint32_t>(left)) ^ (static_cast<uint32_t>(bottom) << 16 | static_cast<uint32_t>(right)); }
		void width(int16_t width) { right += width - right - left; }
		void height(int16_t height) { right += height - bottom - top; }
		Rect() : top(0), left(0), right(0), bottom(0) {}
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

	class Window
	{
		static constexpr short NO_CHANGE = (short)-1;
		static constexpr CharInfo DefaultAttributes = CharInfo(' ', Color::Gray, Color::Black);
		Point _size;
		struct Range { 
			short first; 
			short last; 
			constexpr Range() : first(-1), last(-1) {} 
			template<typename T>
			constexpr Range(T first, T last) : first(static_cast<T>(first)), last(static_cast<T>(last)) {}
			

		};
		struct Line {
			short first;
			short last;
			std::vector<CharInfo> chars;
			explicit Line(size_t size, const CharInfo& fill = DefaultAttributes) : first(0), last(static_cast<short>(size - 1)), chars(size, fill) {}
			void clear(const CharInfo& fill = DefaultAttributes) {  for (size_t i = 0; i < chars.size(); i++) set(i, fill); }
			size_t size() const { return chars.size(); }
			const CharInfo& get(int x) const { return chars.at(x); }
			const CharInfo& operator[](int x) const { return chars.at(x); }
			void set(size_t x, const CharInfo& n) {
				auto& c = chars.at(x);
				short pos = static_cast<short>(x);
				if (c != n) {
					if (first == NO_CHANGE)
						first = last = pos;
					else if (pos < first)
						first = pos;
					else if (pos > last)
						last = pos;
				}
				c = n;
			}
			void touchline() { first = 0; last = static_cast<short>(chars.size() - 1); }
			void untouchline() { last = first = NO_CHANGE;}
			bool touched() const { return first != NO_CHANGE; }
		};
		
		std::vector<Line> _lines;
		CharInfo _default;
		CharInfo _current;
		typedef std::vector<CharInfo>::iterator iterator;
		typedef std::vector<CharInfo>::const_iterator const_iterator;
		Point _cursor;
	public:
		Window();
		Window(int width, int height);
		Window(int width, int height, const CharInfo& default);
		Window(int width, int height, Color fg, Color bg);
		void clear();
		int16_t width() const { return _size.x; }
		int16_t height() const { return _size.y; }
		int16_t row() const { return _cursor.y; }
		int16_t col() const { return _cursor.x; }
		void row(int r) { _cursor.y = r >= _size.y ? _size.y - 1 : r; }
		void col(int c) { _cursor.x = c >= _size.x ? _size.x - 1 : c; }
		std::pair<int16_t, int16_t> cursor() const { return std::make_pair(_cursor.x, _cursor.y); }
		void cursor(int x, int y) { row(y); col(x); }
		const CharInfo& at(size_t x, size_t y) const { return _lines[y][x]; }
		const CharInfo& at() const { return _lines[_cursor.y][_cursor.x]; }
		void scroll(int i);
		void touchwin() { for (auto& line : _lines) line.touchline(); }
		
		void untouchwin() { for (auto& line : _lines) line.untouchline(); }
		void touchline(int start, int count, bool changed) { for (auto& line : _lines) if (changed) line.touchline(); else line.untouchline(); }
		void touchline(size_t start, size_t count) { touchline(start, count, true); }
		bool is_linetouched(int line) const { return _lines.at(line).touched(); }
		bool is_wintouched() const { for (auto& line : _lines) if (line.touched()) return true; return false; }
		void background(Color color) { _current.bg(color); }
		void foreground(Color color) { _current.fg(color); }
		Color background() const { return _current.bg(); }
		Color foreground() const { return _current.fg(); }
		void putch(chtype i);
		void putstr(const char* str, size_t len) {
			while (len--) putch(*str++);
		}
		void putstr(const char* str) {
			while (*str) putch(*str++);
		}
		template<typename T>
		void putstr(const std::basic_string<T>& str) {
			for (auto c : str) putch(c);
		}
		// refresh window to console
		void refresh(size_t x, size_t y,bool clearall=false);

		void print(const char* fmt, ...);
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
	void gotoxy(const Point& p);
	void gotoxy(int x, int y);
	void gotox(int x);
	void gotoy(int y);
	Point cursor();
	void scroll(int lines);
	void scroll_up(int start, int end);
	void scroll_down(int start, int end);
	void cls(int i=2);
	void mode(Mode m);
	void background(Color c);
	void foreground(Color c);

	void init();
//http://minnie.tuhs.org/cgi-bin/utree.pl?file=4.4BSD/usr/src/lib/libcurses/refresh.c

	std::ostream& vt100(); // stream for vt100 emulation on console, only simple escape codes are supported however
	void test_vt(const std::string& text);
	class window : public std::basic_ostream<char>
	{
		std::unique_ptr<std::streambuf> _handle;
	public:
		window();
		window(const Point& p, const Point& s);
		//virtual ~window();
		Point pos() const;
		void pos(const Point& p);
		Point size() const;
		void size(const Point& p);

	};
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

};