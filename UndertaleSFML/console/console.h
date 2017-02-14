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
#include <stack>

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
namespace enum_priv {
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
#define ENUM_OPERATIONS(T) \
	inline constexpr T operator|(T l, T r) { return enum_priv::to_enum<T>(enum_priv::to_underlying(l) | enum_priv::to_underlying(r)); } \
	inline constexpr T operator&(T l, T r) { return enum_priv::to_enum<T>(enum_priv::to_underlying(l) & enum_priv::to_underlying(r));  } \
	inline T& operator|=(T& l, T r) { l = l | r; return l; } \
	inline T& operator&=(T& l, T r) { l = l & r; return l; } \
	inline constexpr bool operator%(T l, T r) { return (enum_priv::to_underlying(l) & enum_priv::to_underlying(r)) != 0; }\
	inline constexpr T operator~(T r) { return enum_priv::to_enum<T>(~enum_priv::to_underlying(r)); }

};
namespace console {
	void init();
	
	// use % to test if the flag is inside the other
 
	
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

	typedef unsigned int chtype;
	struct CharInfo {
		union {
			wchar_t unicode;
			char anscii;
		};
		uint16_t attrib;
		constexpr explicit CharInfo() : unicode(' '), attrib(0xFFFF) {}
		constexpr explicit CharInfo(wchar_t ch) : unicode(ch), attrib(0xFFFF) {}
		constexpr explicit CharInfo(wchar_t ch, uint16_t attrib) : unicode(ch), attrib(attrib) {}
		constexpr bool default_attrib() const { return attrib == 0xFFFF; }
		constexpr operator const wchar_t&() const { return unicode; }
		constexpr operator const char&() const  { return anscii; }
		operator wchar_t&() { return unicode; }
		operator char&() { return anscii; }
		CharInfo& operator=(wchar_t ch) { unicode = ch; return *this; }
		//  with black background and white forground
		Color fg() const { return static_cast<Color>(attrib & 0xF); }
		Color bg() const { return static_cast<Color>((attrib >> 4) & 0xF); }
		void fg(Color c) { attrib = (attrib & 0xF0) | static_cast<uint8_t>(c); }
		void bg(Color c) { attrib = (attrib & 0x0F) | (static_cast<uint8_t>(c) << 4); }
	};
	inline bool operator==(const CharInfo &l, const CharInfo &r) { return l.attrib == r.attrib; }
	inline bool operator!=(const CharInfo &l, const CharInfo &r) { return l.attrib != r.attrib; }
	inline bool operator==(const CharInfo &l, char r) { return l.anscii == r; }
	inline bool operator!=(const CharInfo &l, char r) { return l.anscii != r; }
	inline bool operator==(const CharInfo &l, wchar_t r) { return l.unicode == r; }
	inline bool operator!=(const CharInfo &l, wchar_t r) { return l.unicode != r; }


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
	//	bool contains(const Point& p) const { return contains(p.x, p.y); }
	//	//bool intersects(const Rect& rect, Rect& overlap) const {
	//		overlap = Rect(console::max(left, rect.left), console::max(top, rect.top),
		//		console::min(right, rect.right), console::min(bottom, rect.bottom));
			// If overlapping rect is valid, then there is intersection
	//		return ((overlap.left < overlap.right) && (overlap.top < overlap.bottom));
	//	}
	//	bool intersects(const Rect& rect) const { Rect overlap; return intersects(rect, overlap); }
	};
	inline bool operator==(const Rect &l, const Rect &r) { return l.top == r.top && l.left == r.left && l.right == r.right&& l.bottom == r.bottom; }
	inline bool operator!=(const Rect &l, const Rect &r) { return !(l == r); }
	inline Rect operator*(const Rect &l, const int16_t &r) { return Rect(l.top * r, l.left * r, l.right * r, l.bottom * r); }
	inline Rect operator/(const Rect &l, const int16_t &r) { return Rect(l.top / r, l.left / r, l.right / r, l.bottom / r); }
	inline Rect operator+(const Rect &l, const Rect &r) { return Rect(l.top + r.top, l.left + r.left, l.right + r.right, l.bottom + r.bottom); }
	inline Rect operator-(const Rect &l, const Rect &r) { return Rect(l.top - r.top, l.left - r.left, l.right - r.right, l.bottom - r.bottom); }
	inline Rect operator+(const Rect &l, const Point &r) { return Rect(l.top + r.y, l.left + r.x, l.right + r.x, l.bottom + r.y); }
	inline Rect operator-(const Rect &l, const Point &r) { return Rect(l.top - r.y, l.left - r.x, l.right - r.x, l.bottom - r.y); }

	class VT00WindowBuffer;
	class VT00Window {
		VT00WindowBuffer* _buffer;
		std::iostream _stream;
	public:
		VT00Window(int x, int y, int width, int height);
		VT00Window(const VT00Window& window);
		VT00Window& operator=(const VT00Window& window);
		inline operator std::ostream&() { return _stream; }

		virtual ~VT00Window();
		void put(char i) { _stream.put(i); }
		void put(const char* str, size_t len) { _stream.write(str,len); }
		void put(const char* str) { _stream.write(str, std::strlen(str)); }
		void print(const char* fmt, ...);
		void paint();
		bool scroll() const;
		void scroll(bool v);
	};

	// kind of a hack
	template<typename T>
	VT00Window& operator<< (VT00Window& window, const T& data)
	{
		std::ostream& stream = window;
		stream << data;
		return window;
	}

};


namespace con {
	class VWindow {
		size_t _begx, _begy;
		size_t _curx, _cury;
		size_t _maxx, _maxy;
		std::stringstream _buffer;
		size_t _attrib;
		bool _need_refresh;
		void _refresh();
	public:
		VWindow(size_t width, size_t height);
		VWindow(size_t x, size_t y, size_t width, size_t height);
		void background(console::Color color);
		void forground(console::Color color);
		void refresh(bool clearall=false);
		void scroll(int i);
		void linefeed();
		void move(size_t x, size_t y);
		void putc(char ch);
		void erase_to_eol();
		void erase_to_bot();
		void erase() { move(0, 0); erase_to_bot(); }
		void puts(const char* str) { while (*str) putc(*str++); }
		void print(const char* fmt, ...);
	};
	class Window {
	public:
		/*
		* psuedo functions for standard screen
		*/
		virtual void addch(int ch) = 0;
		virtual void inch(int ch) = 0;
		virtual void clrtoeol() = 0;
	//	virtual char getch() = 0;
		virtual void clrtobot() = 0;
	//	virtual void clearok() = 0;
		virtual void addstr(const std::string& str) { for (auto& c : str) addch(c); }
		//virtual void getstr(std::string& str) = 0;
		virtual void move(size_t y, size_t x) = 0;
		virtual void syncup() =0;   // causes a touchwin() of all of the window's parents.
		inline void mvaddch(size_t y, size_t x, int ch) { move(y, x); addch(ch); }
		inline void mvinsch(size_t y, size_t x, int ch) { move(y, x); inch(ch); }
		
		virtual inline void erase() { move(0, 0); clrtobot(); }
		//virtual void clear() { clearok(); erase(); }
		virtual void syncok(bool bf) = 0;
		virtual void cursyncup() = 0;
		virtual void syncdown() = 0;
		virtual void insertln() = 0;
		virtual void deleteln() = 0;
		virtual void refresh() = 0;
		virtual void update(bool clearall = false) = 0;
		virtual void scroll(int n=1) = 0;
		virtual void touchline(int,int, bool) = 0;
		virtual void touchwin() = 0;
		virtual size_t lines() const = 0;
		virtual size_t cols() const = 0;
		virtual std::shared_ptr<Window> subwin(size_t x, size_t y, size_t width, size_t height)=0;
		virtual ~Window() {}
	};
	int LINES();
	int COLS();
	std::shared_ptr<Window> newwin(size_t x, size_t y, size_t width, size_t height);

	static constexpr char ESC = '\x1b'; // escape
	static constexpr char CSI = '['; // escape
	// stream manipulators
	struct save_cursor {};
	struct restor_cursor {};
	enum class  CursorDirection : char {
		Up=0,
		Down,
		Forward,
		Backward,
	};
	struct no_parm_command {
		const char c;
		constexpr explicit no_parm_command(char c) : c(c) {}
	};
	inline std::ostream& operator<<(std::ostream& os, const no_parm_command& c) { os << ESC << c.c;  return os; }
	struct no_parm_csi_command : public no_parm_command {
		constexpr explicit no_parm_csi_command(char c) : no_parm_command(c) {}
	};
	inline std::ostream& operator<<(std::ostream& os, const no_parm_csi_command& c) { os << ESC << CSI << c.c;  return os; }
	namespace aux {
		template<std::size_t...> struct seq {};

		template<std::size_t N, std::size_t... Is>
		struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

		template<std::size_t... Is>
		struct gen_seq<0, Is...> : seq<Is...> {};

		template<class Ch, class Tr, class Tuple, std::size_t... Is>
		void print_tuple(std::basic_ostream<Ch, Tr>& os, Tuple const& t, seq<Is...>) {
			using swallow = int[];
			(void)swallow {
				0, (void(os << (Is == 0 ? "" : ";") << std::get<Is>(t)), 0)...
			};
		}
	} // aux::
	template<typename...Args>
	struct parm_csi_command {
		const char c;
		std::tuple<Args...> parms;
		//template<typename...cArgs>
		//parm_csi_command(char c, cArgs&&... cargs) : c(c), parms(std::forward<cArgs>(cargs..)) {}
	//	parm_csi_command(char c, Args&&... cargs) : c(c), parms(std::forward<Args>(cargs..)) {}
		parm_csi_command(char c, std::tuple<Args...>&& t) : c(c), parms(t) {}
		template<typename T1>
		parm_csi_command(char c, T1&& t) : c(c), parms(t) {}
		template<typename T1,typename T2>
		parm_csi_command(char c, T1&& t1, T2&& t2) : c(c), parms(t1,t2) {}
	};

	template<class Ch, class Tr, typename...Args>
	auto operator<<(std::basic_ostream<Ch, Tr>& os, parm_csi_command<Args...> const& t) -> std::basic_ostream<Ch, Tr>&
	{
		os << ESC << CSI;
		aux::print_tuple(os, t.parms, aux::gen_seq<sizeof...(Args)>());
		os << t.c;
		return os;
	}
	struct gotoxy : public parm_csi_command<int, int> {
		gotoxy(int x, int y) : parm_csi_command('H', y,x) {}
	};
	struct gotoy : public parm_csi_command<int> {
		gotoy(int y) : parm_csi_command('d', y) {}
	};
	struct gotox : public parm_csi_command<int> {
		gotox(int x) : parm_csi_command('G', x) {}
	};
	struct cursor_up : public parm_csi_command<int> { cursor_up(int count = 1) : parm_csi_command('A', count) {} };
	struct cursor_down : public parm_csi_command<int> { cursor_down(int count = 1) : parm_csi_command('B', count) {} };
	struct cursor_left : public parm_csi_command<int> { cursor_left(int count = 1) : parm_csi_command('C', count) {} };
	struct cursor_right : public parm_csi_command<int> { cursor_right(int count = 1) : parm_csi_command('D', count) {} };

	struct show_cursor {};
	struct hide_cursor {};
	inline std::ostream& operator<<(std::ostream& os, const show_cursor& c) { os << "\x1b[?25h";  return os; }
	inline std::ostream& operator<<(std::ostream& os, const hide_cursor& c) { os << "\x1b[?25l";  return os; }
	struct next_line : public parm_csi_command<int> {
		next_line(int v = 1) : parm_csi_command('E', v) {}
	};
	struct prev_line : public parm_csi_command<int> {
		prev_line(int v = 1) : parm_csi_command('F', v) {}
	};
	struct cls { }; // form feed is clear?
	inline std::ostream& operator<<(std::ostream& os, const cls& l) { os << '\f'; return os; }
	struct _clear_line_from_cursor {};
	struct _clear_line_to_cursor {};
	struct _clear_line {};
	extern _clear_line_from_cursor clear_line_from_cursor;

	inline std::ostream& operator<<(std::ostream& os, const _clear_line_from_cursor& c) { os << "\x1b[0K";  return os; }
	inline std::ostream& operator<<(std::ostream& os, const _clear_line_to_cursor& c) { os << "\x1b[2K";  return os; }
	inline std::ostream& operator<<(std::ostream& os, const _clear_line& c) { os << "\x1b[2K";  return os; }
	struct clear_window_from_cursor {};
	struct clear_window_to_cursor {};
	struct clear_window {};
	inline std::ostream& operator<<(std::ostream& os, const clear_window_from_cursor& c) { os << "\x1b[0J";  return os; }
	inline std::ostream& operator<<(std::ostream& os, const clear_window_to_cursor& c) { os << "\x1b[2J";  return os; }
	inline std::ostream& operator<<(std::ostream& os, const clear_window& c) { os << "\x1b[2J";  return os; }

	struct print {
		std::string str;
		print(const char* fmt, ...);
	};
	inline std::ostream& operator<<(std::ostream& os, const print& c) { os << c.str;  return os; }
};


namespace logging {

};