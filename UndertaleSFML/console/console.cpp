#include <windows.h>

#include "console.h"
#include <queue>
#include <stack>

inline COORD& operator+=(COORD& l, const COORD& r) { l.X -= r.X; l.Y -= r.Y; return l; }
inline COORD& operator-=(COORD& l, const COORD& r) { l.X -= r.X; l.Y -= r.Y; return l; }
inline COORD& operator*=(COORD& l, SHORT r) { l.X *= r; l.Y *= r; return l; }
inline COORD operator-(const COORD& l) { return COORD{ -l.X, -l.Y }; }
inline COORD operator+(const COORD& l, const COORD& r) { return COORD{ l.X + r.X, l.Y + r.Y }; }
inline COORD operator-(const COORD& l, const COORD& r) { return COORD{ l.X - r.X, l.Y - r.Y }; }
inline COORD operator*(const COORD& l, SHORT r) { return COORD{ l.X * r, l.Y * r }; }
inline COORD operator*(SHORT r, const COORD& l) { return COORD{ l.X * r, l.Y * r }; }

inline COORD& operator*=(COORD& l, const COORD& r) { l.X *= r.X; l.Y *= l.Y; return l; }

#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %d from %s on line %d\n", __FILE__, GetLastError(), api, __LINE__);}
namespace {
	void 	init_cerr();
	void init_cout();
};
namespace console {
	void init() {
		DWORD dwMode = 0;
		GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
	//	if (_scroll) dwMode |= ENABLE_WRAP_AT_EOL_OUTPUT;
		SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode);
		init_cerr();
	//	init_cout();
	}
	enum class State {
		Ground,
		Escape,
		Csi,
		Parm,
		Ignore,
		Execute,
		Dispatch,
	};
	struct escape_decode {
		bool csi;
		wchar_t postfix;
		std::array<int, 16> parms;
		int parm_count;
		std::wstring inside;
		State state;
		void clear() {
			csi = false;
			state = State::Ground;
			postfix = '\0';
			parms.fill(0);
			inside.clear();
			parm_count = 0;
		}
		escape_decode() { clear(); }
		State putch(wchar_t ch) { // does NOT try to detect invalid sequences
			if ((ch >= 0x00 && ch <= 0x17) || ch == 0x19 || (ch >= 0x1C && ch <= 0x1F) || ch == 0x7F)
				return state; // ignore control stuff
			switch (ch) {
			case '\x1b':
				clear();
				state = State::Escape;
				return state; // we always reset
			}
			if (state == State::Ground) return state;// short
			while (true) {
				switch (state) {
				case State::Escape:
					if (ch == '[') {
						csi = true;
						state = State::Csi;
					}
					else if (ch >= 0x20 && ch <= 0x2F) {
						inside.push_back(ch);
					}
					else if (ch >= 0x30 && ch <= 0x7E) {
						postfix = ch;
						state = State::Dispatch;
						continue;
					}
					break;
				case State::Parm:
					if (ch >= 0x30 && ch <= 0x39)
						parms[parm_count] = (parms[parm_count] * 10) + ch - '0x30';
					else if (ch == ';') parm_count++;
					else {
						state = State::Csi;
						continue; // not really true, but we let csi handle eveything else
					}
					break;
				case State::Csi:
					if (ch >= 0x30 && ch <= 0x39) {
						state = State::Parm;
						continue;
					} if ((ch >= 0x40 && ch <= 0x7E)) {
						postfix = ch;
						state = State::Dispatch;
						continue;
					}
					else inside.push_back(ch);
					break;
				}
				return state;
			}
		}
	};
	// we want the stream using wchar_t so we can access the graphic charaters of
	// unicode that windows has on the console
	class VT00WindowBuffer : public std::streambuf {
		std::array<wchar_t, 512> _console_temp;
		std::array<char, 256> _output;
		std::array<char, 256> _input;
		CONSOLE_SCREEN_BUFFER_INFO info;
		HANDLE _handle;
		bool _scroll;
		int _x, _y, _width, _height;
		mutable std::vector<CHAR_INFO> _screen_buffer; 
		std::vector<wchar_t> _buffer;
		escape_decode _esc;
		inline wchar_t tryIndex(size_t i) const  { return _buffer.size() < i ? _buffer[i] : 0; }
	protected:
		void reset_esc() {
			_buffer.clear();
		}
		void output_buffer() {
			if (_buffer.size() > 0) {
				DWORD written;
				::WriteConsoleW(_handle, _buffer.data(), _buffer.size(), &written, NULL);
				_buffer.clear();
			}
			
		}
		void handle_esc() {
		
			
		}
		void push(wchar_t ch) {
			auto state = _esc.putch(ch);
			if (state == State::Ground) {
				_buffer.push_back(ch);
			} else {
				if (state == State::Escape) output_buffer(); // flush first
				_buffer.push_back(ch);
				if (state == State::Dispatch) handle_esc();
			}
		}
		int sync() override {
			std::ptrdiff_t n = pptr() - pbase();
			int real_n = ::MultiByteToWideChar(CP_UTF8, 0, pbase(), n, 0, 0);
			::MultiByteToWideChar(CP_UTF8, 0, pbase(), n, _console_temp.data(), real_n);
			DWORD written;
			::WriteConsoleW(_handle, _console_temp.data(), real_n, &written, NULL);
			//::WriteConsoleA(_handle, pbase(), n, &written, NULL);
			assert(written == real_n);
			pbump(-n);
			return 0;
		}
		void update_handle() {
			if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) 
				_handle = ::CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
			assert(_handle != INVALID_HANDLE_VALUE);
			DWORD dwMode = 0;
			GetConsoleMode(_handle, &dwMode);
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
			if (_scroll) dwMode |= ENABLE_WRAP_AT_EOL_OUTPUT;
			SetConsoleMode(_handle, dwMode);
			SMALL_RECT rect = { 0,0, short(_width - 1),short(_height - 1) };
			SetConsoleWindowInfo(_handle, TRUE,&rect);
			GetConsoleScreenBufferInfo(_handle, &info);
			_screen_buffer.resize(_width*_height);
		}
	public:
		bool scroll() const { return _scroll; }
		void scroll(bool value) {
			if (value != _scroll) {
				_scroll = value;
				update_handle();
			}
		}
		VT00WindowBuffer(int x, int y, int width, int height) :_x(x),_y(y),_width(width),_height(height), _scroll(true), _handle(nullptr) {
			// configure handle
			update_handle();
			// configure charater buffer
			setg(&_input.front(), &_input.front(), &_input.back() - 1);
			setp(&_output.front(), &_output.back() - 1);
		}
		virtual ~VT00WindowBuffer() {
			if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) return;
			::CloseHandle(_handle);
			_handle = nullptr;
		}
		VT00WindowBuffer(VT00WindowBuffer&& move) = default;
		VT00WindowBuffer& operator=(VT00WindowBuffer&& move) = default;
		VT00WindowBuffer(const VT00WindowBuffer& copy) :_x(copy._x), _y(copy._y), _width(copy._width), _height(copy._height), _scroll(copy._scroll), _handle(nullptr) {
			update_handle();
			// configure charater buffer
			setg(&_input.front(), &_input.front(), &_input.back() - 1);
			setp(&_output.front(), &_output.back() - 1);
		}
		VT00WindowBuffer& operator=(const VT00WindowBuffer& copy) {
			sync(); // flush
			_x = (copy._x); _y = (copy._y); _width = (copy._width); _height = (copy._height); _scroll = (copy._scroll);
			update_handle();
			return *this;
		}
		
		void refresh()   {
			sync();
			// refresh window
			COORD size = { short(_width),short(_height) };
			COORD pos = { 0,0 };
			SMALL_RECT rect = { 0, 0, short(_width - 1), short(_height - 1) };
			ReadConsoleOutputA(_handle, _screen_buffer.data(), size, pos, &rect);
			rect.Left = _x;
			rect.Right = _x + _width;
			rect.Top = _y;
			rect.Bottom = _y + _height;
			WriteConsoleOutputA(GetStdHandle(STD_OUTPUT_HANDLE), _screen_buffer.data(), size, pos, &rect);
		}
	};
	VT00Window::VT00Window(int x, int y, int width, int height) : _buffer(new VT00WindowBuffer(x,y,width,height)), _stream(_buffer) { }
	VT00Window::VT00Window(const VT00Window& window) : _buffer(new VT00WindowBuffer(*window._buffer)), _stream(_buffer) { }
	VT00Window& VT00Window::operator=(const VT00Window& copy) {
		*_buffer = *copy._buffer;
		return *this;
	}
	VT00Window::~VT00Window() {
		if (_buffer != nullptr) {
			delete _buffer;
			_buffer = nullptr;
		}
	}
	void VT00Window::paint() {
		//_buffer->paint();
	}
	bool VT00Window::scroll() const { return _buffer->scroll(); }
	void VT00Window::scroll(bool v) { _buffer->scroll(v); }
	void VT00Window::print(const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		char buffer[256];
		auto count = vsnprintf(buffer, 256, fmt, va);
		va_end(va);
		_stream.write(buffer, count);
	}
};

namespace con {
	_clear_line_from_cursor clear_line_from_cursor;
		print::print(const char* fmt, ...) {
			va_list va;
			va_start(va, fmt);
			char buffer[256];
			auto count = vsnprintf(buffer, 256, fmt, va);
			va_end(va);
			str.assign(buffer, buffer + count);
		}
	/*

	std::ostream& operator<<(std::ostream& os, const cls& l) { console::cls(); return os; }
	std::ostream& operator<<(std::ostream& os, const background& l) { console::background(l.c); return os; }
	std::ostream& operator<<(std::ostream& os, const foreground& l) { console::foreground(l.c); return os; }
	std::ostream& operator<<(std::ostream& os, const mode& l) { console::mode(l.m); return os; }
	std::ostream& operator<<(std::ostream& os, const gotoxy& l) { console::gotoxy(l.p); return os; }
	std::ostream& operator<<(std::ostream& os, const gotox& l) { console::gotox(l.x); return os; }
	std::ostream& operator<<(std::ostream& os, const gotoy& l) { console::gotoy(l.y); return os; }
	*/
};
template<class T>
struct is_c_str
	: std::integral_constant<
	bool,
	std::is_same<char *, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value ||
	std::is_same<char const *, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value
	> {};
template<class T>
struct is_c_wstr
	: std::integral_constant<
	bool,
	std::is_same<wchar_t *, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value ||
	std::is_same<wchar_t const *, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value
	> {};
// all from http://www.codeproject.com/Articles/1053/Using-an-output-stream-for-debugging
// https://cdot.senecacollege.ca/software/nled/nled_2_52_src/qkdisp.c
// alot of helper functions up there
template<class CharT, class TraitsT = std::char_traits<CharT> >
class visual_studio_debugbuf : public std::basic_streambuf<CharT, TraitsT> {
	std::array<CharT, 512> _buffer;
	std::vector<CharT> _linebuffer;
	int _lastc;
protected:
	template<typename T>
	typename std::enable_if<is_c_str<T>::value,void>::type
	 output_visualstudio(T str) {
		::OutputDebugStringA(str);
	}
	template<typename T>
	typename std::enable_if<is_c_wstr<T>::value, void>::type
	 output_visualstudio(T str) {
		::OutputDebugStringW(str); 
	}
	int sync() override
	{
		std::ptrdiff_t n = pptr() - pbase();

		for (auto ptr = pbase(); ptr != pptr(); ptr++) {
			CharT ch = *ptr;
			if (ch == '\r' || ch == '\n') {
				if (_lastc != ch && (_lastc == '\r' || _lastc == '\n')) {
					ch = 0;
				}
				else {
					_linebuffer.push_back(0);
					auto now = std::chrono::system_clock::now();
					auto in_time_t = std::chrono::system_clock::to_time_t(now);
					std::basic_stringstream<CharT, TraitsT> ss;
					ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]:" << _linebuffer.data() << std::endl;
					output_visualstudio(ss.str().c_str());
					_linebuffer.clear();
				}
			}
			else _linebuffer.push_back(ch);
			_lastc = ch;
		}
		pbump(-n);
		return 0;
	}
public:
	visual_studio_debugbuf() :  std::streambuf(), _lastc(0) {
		char *base = _buffer.data();
		setp(base, base + _buffer.size() - 1); // -1 to make overflow() easier
	}

};


visual_studio_debugbuf<char> s_cerr_debug_buffer;
//basic_debugbuf<char> s_cout_debug_buffer(true, false);

namespace  {
	void init_cerr() {
	//	s_cerr_debug_buffer.setOldBuffer(std::cerr.rdbuf());
		std::cerr.rdbuf(&s_cerr_debug_buffer);
		std::cerr << "cerr redirected" << std::endl;
	}
	void init_cout() {
	//	s_cout_debug_buffer.setOldBuffer(std::cout.rdbuf());
		//std::cout.rdbuf(&s_cout_debug_buffer);
		std::cout << "cout redirected" << std::endl;
	}
	void error(const std::string& message) {

	}
};