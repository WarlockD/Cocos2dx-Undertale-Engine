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
	const Point Point::Up(0, -1);
	const Point Point::Down(0, 1);
	const Point Point::Left(-1, 0);
	const Point Point::Right(1, 0);
	

	namespace _details
	{

		void _settextcolor(HANDLE const console, console::Color const color)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);

			WORD attr = info.wAttributes & 0xFFF0;
			attr |= static_cast<WORD>(color);

			::SetConsoleTextAttribute(console, attr);
		}

		void _setbgcolor(HANDLE const console, console::Color const color)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);

			WORD attr = info.wAttributes & 0xFF0F;
			attr |= static_cast<WORD>(color) << 4;

			::SetConsoleTextAttribute(console, attr);
		}

		void _setcolors(HANDLE const console, console::Color const fg, console::Color const bg)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);

			WORD attr = info.wAttributes & 0xFF00;
			attr |= static_cast<WORD>(fg);
			attr |= static_cast<WORD>(bg) << 4;
			::SetConsoleTextAttribute(console, attr);
		}

		void _setmode(HANDLE const console, console::Mode modes)
		{
			DWORD mode = (0x1FF & static_cast<DWORD>(modes)) | ::GetConsoleMode(console, &mode);
			if (modes % Mode::Override) { // special case for override
				mode &= ~ENABLE_INSERT_MODE;
				mode |= ENABLE_EXTENDED_FLAGS;
			}
			::SetConsoleMode(console, mode);
		}

		void _clearmode(HANDLE const console, Mode const modes)
		{
			DWORD mode = ::GetConsoleMode(console, &mode) & ~(0x1FF & static_cast<DWORD>(modes));
			if (modes % Mode::Override) { // special case for override
				mode |= (ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE);
			}
			::SetConsoleMode(console, mode);
		}

		int _getposx(HANDLE const console)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);
			return info.dwCursorPosition.X;
		}
		ScreenInfo getScreenInfo(HANDLE const console) {
			ScreenInfo info;
			::GetConsoleScreenBufferInfo(console, reinterpret_cast<CONSOLE_SCREEN_BUFFER_INFO*>(&info));
			return info;
		}

		void _setposx(HANDLE const console, int const x)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);
			info.dwCursorPosition.X = x;
			::SetConsoleCursorPosition(console, info.dwCursorPosition);
		}

		int _getposy(HANDLE const console)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);
			return info.dwCursorPosition.Y;
		}

		void _setposy(HANDLE const console, int const y)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);
			info.dwCursorPosition.Y = y;
			::SetConsoleCursorPosition(console, info.dwCursorPosition);
		}

		console::Point _getpos(HANDLE const console)
		{
			CONSOLE_SCREEN_BUFFER_INFO info;
			::GetConsoleScreenBufferInfo(console, &info);
			return console::Point(info.dwCursorPosition.X, info.dwCursorPosition.Y);
		}

		void _setpos(HANDLE const console, console::Point const pos)
		{
			::SetConsoleCursorPosition(console, reinterpret_cast<const COORD&>(pos));
		}
		size_t _writeConsole(HANDLE const console, const char* str, size_t size) {
			DWORD written;
			::WriteConsoleA(console, str, size, &written, NULL);
			return written;
		}
		size_t _readConsole() {
			//::ReadConsoleA
			return 0;
		}
	}
	const CharInfo CharInfo::Blank(' ', Color::White, Color::Black);
	static HANDLE hConsole = NULL;
	static std::vector<CHAR_INFO> console_buffer;
	static COORD console_buffer_size;
	static SMALL_RECT console_window;
	static CONSOLE_SCREEN_BUFFER_INFO console_info;
	static constexpr COORD ZERO_COORD = { (SHORT)0, (SHORT)0 };


	void output_context::restore() {
		HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		::SetConsoleCursorPosition(handle, *reinterpret_cast<COORD*>(&_info.cursor));
		::SetConsoleTextAttribute(handle, _info.attrib);
		::SetConsoleMode(handle, to_underlying(_mode));
		//::SetConsoleScreenBufferSize(handle, scrbuf_info.dwSize);
	}
	void output_context::save() {
		HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		::GetConsoleScreenBufferInfo(handle, reinterpret_cast<PCONSOLE_SCREEN_BUFFER_INFO>(&_info));
		::GetConsoleMode(handle, reinterpret_cast<DWORD*>(&_mode));
	}


	void redraw() {
		if (hConsole && console_buffer.size() > 0) {
			SMALL_RECT rect = console_window;
			PERR(WriteConsoleOutput(hConsole, console_buffer.data(), console_buffer_size, ZERO_COORD, &rect), "SetConsoleScreenBufferInfoEx");
		}
	}

	void init() {
		if (hConsole) return;

	    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		init_cerr();
		init_cout();
		//	PERR(hConsole, "GetStdHandle");
		//	PERR(GetConsoleScreenBufferInfo(hConsole, &console_info), "GetConsoleScreenBufferInfo");
		//	console_window = console_info.srWindow;
		//	console_buffer_size = { console_window.Right - console_window.Left , console_window.Bottom - console_window.Top };
		//	console_buffer.resize(console_buffer_size.X * console_buffer_size.Y, { ' ', console_info.wAttributes });
	}
	/* Standard error macro for reporting API errors */
	void gotoxy(int x, int y) {
		_details::_setpos(hConsole, Point(x, y));
	}
	void gotoxy(const Point& p) {
		_details::_setpos(hConsole, p);
	}
	void gotox(int x) {
		_details::_setposx(hConsole, x);
	}
	void gotoy(int y) {
		_details::_setposy(hConsole, y);
	}
	Point cursor() {
		return _details::_getpos(hConsole);
	}
	void mode(Mode m) {
		_details::_setmode(hConsole, m);
	}
	void background(Color c) {
		_details::_settextcolor(hConsole, c);
	}
	void foreground(Color c) {
		_details::_setbgcolor(hConsole, c);
	}
	//http://stackoverflow.com/questions/13003645/stdfunction-as-a-custom-stream-manipulator
	// mabey set it up like that
	Point screen_size() {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		PERR(GetConsoleScreenBufferInfo(hConsole, &csbi), "GetConsoleScreenBufferInfo");
		return Point(csbi.dwSize.X, csbi.dwSize.Y);
	}
	void cls(int i) {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		PERR(GetConsoleScreenBufferInfo(hConsole, &csbi), "GetConsoleScreenBufferInfo");
		COORD start;
		DWORD count;
		switch (i) {
		case 1:
			start = COORD{ 0,0 };
			count = csbi.dwSize.X + (csbi.dwSize.X * (start.Y - 1));
			break;
		case 2:start = COORD{ 0,0 }; count = csbi.dwSize.X* csbi.dwSize.Y; break;
		case 0:
		default: // 0
			start = csbi.dwCursorPosition;
			count = csbi.dwSize.X - csbi.dwCursorPosition.X + (csbi.dwSize.X * (csbi.dwSize.Y - start.Y));
			assert(i == 0);
			break;
		}
		/* fill the entire screen with blanks */
		DWORD written;
		PERR(FillConsoleOutputCharacterA(hConsole, ' ', count, start, &written), "FillConsoleOutputCharacter");
		PERR(FillConsoleOutputAttribute(hConsole, csbi.wAttributes, count, start, &written), "FillConsoleOutputCharacter");
	}
	void scroll(int lines) {
		if (lines == 0) return; // no lines to scroll;
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
		PERR(GetConsoleScreenBufferInfo(hConsole, &csbiInfo), "GetConsoleScreenBufferInfo");
		SMALL_RECT srctScrollRect, srctClipRect;
		COORD coordDest = { 0 ,0 };
		CHAR_INFO chiFill{ (char)' ', 7 };
		srctScrollRect.Top = lines < 0 ? -lines : 0;
		srctScrollRect.Bottom = csbiInfo.dwSize.Y - (lines < 0 ? 1 : lines);
		srctScrollRect.Left = 0;
		srctScrollRect.Right = csbiInfo.dwSize.X - 1;
		coordDest.X = 0;
		coordDest.Y = 0;
		srctClipRect = srctScrollRect;
		PERR(ScrollConsoleScreenBuffer(hConsole, &srctScrollRect, &srctClipRect, coordDest, &chiFill), "ScrollConsoleScreenBuffer");
	}

	void scroll_down(int startrow, int endrow) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		PERR(GetConsoleScreenBufferInfo(hConsole, &info), "GetConsoleScreenBufferInfo");
		SMALL_RECT tot, scr;
		COORD dest;
		CHAR_INFO ch;
		tot.Left = 0;
		tot.Right = info.dwSize.X - 1;
		tot.Top = startrow;
		tot.Bottom = endrow;
		scr.Left = 0;
		scr.Right = info.dwSize.X - 1;
		scr.Top = startrow;
		scr.Bottom = endrow - 1;
		dest.X = 0;
		dest.Y = startrow + 1;
		ch.Char.AsciiChar = ' ';
		ch.Attributes = BACKGROUND_GREEN | FOREGROUND_RED;
		ScrollConsoleScreenBuffer(hConsole, &scr, &tot, dest, &ch);
		//touchline(startrow, 0);
	//	touchline(endrow, 0);
	}

	void scroll_up(int startrow, int endrow) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		PERR(GetConsoleScreenBufferInfo(hConsole, &info), "GetConsoleScreenBufferInfo");
		SMALL_RECT tot, scr;
		COORD dest;
		CHAR_INFO ch;
		tot.Left = 0;
		tot.Right = info.dwSize.X - 1;
		tot.Top = startrow;
		tot.Bottom = endrow;
		scr.Left = 0;
		scr.Right = info.dwSize.X - 1;
		scr.Top = startrow + 1;
		scr.Bottom = endrow;
		dest.X = 0;
		dest.Y = startrow;
		ch.Char.AsciiChar = ' ';
		ch.Attributes = BACKGROUND_GREEN | FOREGROUND_RED;
		ScrollConsoleScreenBuffer(hConsole, &scr, &tot, dest, &ch);
		//touchline(startrow, 0);
		//touchline(endrow, 0);
	}

	// putch has a bare bones scrolling interface.  It handles stuff like /n/r /b etc and keeps the cursor within the window

	template<class charT, class traits = std::char_traits<charT> >
	class mystream : public std::basic_ostream<charT, traits>
	{
	public:
		static const int xindex;
		mystream(std::basic_ostream<charT, traits>& ostr) :
			std::basic_ostream<charT, traits>(ostr.rdbuf())
		{
			this->pword(xindex) = this;
		}

		void myfn()
		{
			*this << "[special handling for mystream]";
		}
	};
	// parser is built from the state diagram from here
	// http://vt100.net/emu/dec_ansi_parser
	// used the codes from here, but I could get more in here..humm
	// https://en.wikipedia.org/wiki/ANSI_escape_code
	class VT100 {
		enum class State {
			ground,
			escape,
			escape_intermediate,
			escape_dispatch,
			count_escape,
			csi_entry,
			csi_intermediate,
			csi_parm,
			csi_dispatch,
			csi_ignore,
			count_csi,
			dcs_entry,

			exit
		};

		std::stack<State> _states;
		std::queue<char> _chars;
		State _state = State::ground;
		std::vector<int> _parms;
		int _current_parm = 0;
		std::vector<char> _collect;
		void clear() {
			_parms.clear();// 30 - 39) and the semicolon(code 3B
			_collect.clear();
			_current_parm = 0;
		}
		inline bool is_csi_state() const { return _state >= State::csi_entry && _state <= State::count_csi; }
		bool parm(char c) {
			if (c >= 0x30 && c <= 0x39) {
				_current_parm = (_current_parm * 10) + (0x30 - c);
				return true;
			}
			else if (c == 0x3B) {
				_parms.push_back(_current_parm);
				_current_parm = 0;
				return true;
			}
			return false;
		}
		void collect(char c) {
			_collect.push_back(c);
		}
		bool execute(char c) {

			if ((c >= 0x00 && c <= 0x17) || c == 0x19 || (c >= 0x1C && c <= 0x1F)) {
				return true; // execute, happens in any state
			}

			// we do the execute commands here
			// return if it was executed

			return false;
		}
		void csi_dispatch(char c) {
			CONSOLE_SCREEN_BUFFER_INFO info;
			PERR(GetConsoleScreenBufferInfo(hConsole, &info), "GetConsoleScreenBufferInfo");
			COORD cursor = info.dwCursorPosition;
			switch (c) {
			case 'A': cursor = info.dwCursorPosition + COORD{ 0,-1 } *(_current_parm ? _current_parm : 1); break;
			case 'B': cursor = info.dwCursorPosition + COORD{ 0, 1 } *(_current_parm ? _current_parm : 1); break;
			case 'C': cursor = info.dwCursorPosition + COORD{ 1,0 } *(_current_parm ? _current_parm : 1); break;
			case 'D': cursor = info.dwCursorPosition + COORD{ -1,0 } *(_current_parm ? _current_parm : 1); break;
			case 'E': cursor.X = 0; info.dwCursorPosition.Y += (_current_parm ? _current_parm : 1);  break;
			case 'F': cursor.X = 0; info.dwCursorPosition.Y -= (_current_parm ? _current_parm : 1);  break;
			case 'G': cursor.X = (_current_parm ? _current_parm - 1 : 0);  break;
			case 'H':
				cursor.Y = (_current_parm ? _current_parm - 1 : 0);
				cursor.X = (_parms.size() > 0 && _parms[0] ? _parms[0] - 1 : 0);
				break;
			case'J': // erase display
				assert(false);
				break;

			}
			SetConsoleCursorPosition(hConsole, cursor);
		}
		void escape_dispatch(char c) {

		}
	public:
		void putch(char c) {
			if (_state == State::csi_ignore) {
				if (!execute(c) && c != 0x7f && !(c >= 0x40 && c <= 0x7e))_state = State::ground;
				else return;
			}
			switch (c) {
			case 0x1B: _state = State::escape; clear(); return;// escape
		//	case 0x9D: _state = State::osc_string; return;// escape
			}
			if (is_csi_state()) {
				if (c == 0x7f || execute(c)) return;
				if (c >= 0x40 && c <= 0x7e) _state = State::csi_dispatch;
			}
			while (true) {
				switch (_state) {
				case State::ground:
				{
					char buffer[1] = { c };
					DWORD writin;
					WriteConsoleA(hConsole, &buffer, 1, &writin, NULL);
				}

				break;
				case State::csi_ignore:
					if (!execute(c) && c >= 0x40 && c <= 0x7e) {
						_state = State::ground;
						continue;
					}
					break;
				case State::escape_dispatch:
					escape_dispatch(c);
					_state = State::ground;
					break;
				case State::csi_dispatch:
					csi_dispatch(c);
					_state = State::ground;
					break;
				case State::escape_intermediate:
					if (c >= 0x20 && c <= 0x2F) collect(c);
					else if (c >= 30 && c <= 0x7E) {
						_state = State::escape_dispatch;
						continue;
					}
					break;
				case State::escape:
					switch (c) {
					case 0x5B: _state = State::csi_entry; break;
					default:
						if (c >= 0x20 && c <= 0x2F) {
							_state = State::escape_intermediate;
							continue;
						}
						else if ((c >= 0x30 && c <= 0x4F) || (c >= 0x51 && c <= 0x57) || c == 0x59 || c == 0x5A || c == 0x5C || (c >= 0x60 && c <= 0x7E)) {
							_state = State::escape_dispatch;
							continue;
						}
						assert(false);
					}
					break;
				case State::csi_entry:
					clear();
					if (c >= 0x20 && c <= 0x2F) {
						_state = State::csi_intermediate;
						continue;
					}
					else if (c == 0x3A) _state = State::csi_ignore;
					// We don't know if if its a collect or parm yet so do it then just return
					else if (c >= 0x3C && c <= 0x3F) {
						collect(c); _state = State::csi_parm;
					}
					else if (parm(c)) _state = State::csi_parm;
					break;
				case State::csi_intermediate:
					if (c >= 0x40 && c <= 0x7E) {
						_state = State::csi_dispatch;
						continue;
					}
					else if (c >= 0x30 && c <= 0x3F) {
						_state = State::csi_ignore;
						continue;
					}
					else if (c >= 0x20 && c <= 0x2F) collect(c);
					else {
						assert(false);
					}
					break;
				case State::csi_parm:
					if (c >= 0x20 && c <= 0x2F) {
						_state = State::csi_intermediate;
						continue;
					}
					else if (c == 0x3A || c >= 0x3C && c <= 0x3F)
						_state = State::csi_ignore;
					else if (!parm(c)) {
						assert(false);
					}
					break;
				}
				break;
			}
		}
	};
	std::ostream& vt100() {
		return std::cout;
	} // stream for vt100 emulation on console, only simple escape codes are supported however
	static VT100 vt;
	void test_vt(const std::string& text) {
		for (char c : text)
			vt.putch(c);
	}
	class window_handle : public std::streambuf {
	public:
		void refresh() {

		}
		int sync() override
		{
			std::ptrdiff_t n = pptr() - pbase();
			DWORD dummy;
			::WriteConsoleA(hConsole, pbase(), n, &dummy, NULL);

			
			refresh();
			return 0;
		}
		HANDLE con_buffer;
		COORD pos;
		COORD size;
		std::vector<CHAR_INFO> screen;
		CONSOLE_CURSOR_INFO cursinfo;
		CONSOLE_SCREEN_BUFFER_INFO scrinfo;

		void set_pos(const Point& p) {
			pos = { p.x, p.y };
		}
		void set_size(const Point& s) {
			size = { s.x, s.y };
			SetConsoleScreenBufferSize(con_buffer, size);
			screen.resize(size.X * size.Y);
		}
		char textBuffer[512];
		window_handle() :con_buffer(CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL)) {
			char* base = textBuffer;
			setp(base, base + 512 - 1); // -1 to make overflow() easier
		}
		virtual ~window_handle() { CloseHandle(con_buffer); con_buffer = nullptr; }
	};
	//struct Handle { virtual ~Handle() {} };
	//std::unique_ptr<Handle> _handle;
	window::window() : _handle(new window_handle), std::ostream(_handle.get()) {
		rdbuf(_handle.get());
	}
	window::window(const Point& p, const Point& s) : _handle(new window_handle), std::ostream(_handle.get()) {
		rdbuf(_handle.get());
		pos(p);
		size(s);
	}
	Point  window::pos() const {
		auto ptr = static_cast<window_handle*>(_handle.get());
		return Point(ptr->pos.X, ptr->pos.Y);
	}
	void  window::pos(const Point& p) {
		auto ptr = static_cast<window_handle*>(_handle.get());
		ptr->set_pos(p);
	}
	Point  window::size() const {
		auto ptr = static_cast<window_handle*>(_handle.get());
		return Point(ptr->size.X, ptr->size.Y);
	}
	void  window::size(const Point& p) {
		auto ptr = static_cast<window_handle*>(_handle.get());
		ptr->set_size(p);
	}
	// refresh window to console
	void transform_line(int lineno, int x, int len, const CharInfo *srcp) {
		//CHAR_INFO ci[512];
		const CHAR_INFO* ci = reinterpret_cast<const CHAR_INFO*>(srcp);
		COORD bufPos = { 0, 0 };
		COORD bufSize = { len, 1 };
	//	COORD bufSize, bufPos;
		SMALL_RECT sr = { x, lineno, x + len - 1, lineno };
		/*
		
		for (int j = 0; j < len; j++)
		{
			chtype ch = srcp[j];

			ci[j].Attributes = pdc_atrtab[ch >> PDC_ATTR_SHIFT];
			if (sizeof(chtype) == 4) {
				if (ch & A_ALTCHARSET && !(ch & 0xff80))
					ch = acs_map[ch & 0x7f];
			}
			ci[j].Char.UnicodeChar = ch & A_CHARTEXT;
		}
		WriteConsoleOutput(console::hConsole, ci, bufSize, bufPos, &sr);
		*/
		WriteConsoleOutput(console::hConsole, ci, bufSize, bufPos, &sr);
	}
	
	static bool clearall = false;
	Window con_win;

	void refresh_console() {
		if (con_win.width() == 0) {
			// resize the window
			auto info = _details::getScreenInfo(console::hConsole);
			con_win = Window(info.size.x, info.size.y);
		}
	}
	Window::Window() : _size(0, 0), _cursor(0, 0), _default(' ', Color::Gray, Color::Black) {}
	Window::Window(int width, int height) : _size(width, height), _cursor(0, 0), _default(' ', Color::Gray, Color::Black), _lines(height, Line(width, _default)), _current(_default) {}
	Window::Window(int width, int height, const CharInfo& default) : _size(width, height), _cursor(0, 0), _default(default), _lines(height, Line(width, _default)), _current(_default) {}
	Window::Window(int width, int height, Color fg, Color bg) : _size(width, height), _cursor(0, 0),
		_default(' ', fg, bg), _lines(height, Line(width, _default)), _current(_default) {}

	void Window::refresh(size_t begx, size_t begy, bool clearall) {
		//static std::vector<CharInfo> back_buffer(100 * 200); // stupid back buffer
		refresh_console();
		//if (back_buffer.size() < 100 * 200) back_buffer.resize(100 * 200);
		for (size_t i = 0, j = begy; i < static_cast<size_t>(_size.y); i++, j++)
		{
			auto& line = _lines.at(i);
			if (clearall || line.touched()) {
				short first = clearall ? 0 : line.first, last = clearall ? _size.x - 1 : line.last;
				CharInfo *src = line.chars.data();
				CharInfo *dest = con_win._lines[begy].chars.data() + begx;

				while (first <= last)
				{
					int len = 0;

					/* build up a run of changed cells; if two runs are
					separated by a single unchanged cell, ignore the
					break */

					if (clearall)
						len = last - first + 1;
					else
						while (first + len <= last &&
							(src[first + len] != dest[first + len] ||
							(len && first + len < last &&
								src[first + len + 1] != dest[first + len + 1])
								)
							)
							len++;

					/* update the screen, and pdc_lastscr */

					if (len)
					{
						transform_line(j, first+begx, len, src + first);
						std::memcpy(dest + first, src + first, len * sizeof(chtype));
						first += len;
					}

					/* skip over runs of unchanged cells */

					while (first <= last && src[first] == dest[first])
						first++;
				}
				line.untouchline();
			}
		}
	}
	void Window::print(const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		char buffer[256];
		auto count = vsnprintf(buffer, 256, fmt, va);
		va_end(va);
		putstr(buffer, count);
	}
	void Window::clear() { 
		for (auto& line : _lines) line.clear(_default);
	}
	void Window::scroll(int n) {
		if (n == 0) return; // don't scroll with 0
		std::vector<Line>::iterator start, end;
		int dir = 0;
		/* Check if window scrolls. Valid for window AND pad */
		if (n > 0)
		{

			start = _lines.begin();
			end = _lines.end() - 1;
			dir = 1;
		}
		else
		{
			start = _lines.end() - 1;
			end = _lines.begin();
			dir = -1;
		}
		for (int l = 0; l < (n * dir); l++) {
			auto temp = start;
			for (auto i = start; i != end; i += dir)
				std::swap(*i, *(i + dir));
			std::swap(*end, *temp);
		}
		touchline(0, _size.y - 1); // touch eveything, but change this when we have margens
	}
	void Window::putch(chtype i) {
		switch (i) {
		case '\n':
			_cursor.y++;
			if (_cursor.y >= _size.y) { scroll(1); _cursor.y = _size.y - 1; }
			break;
		case '\r': _cursor.x = 0; break;
		case '\b': if (_cursor.x != 0) { _cursor.x--; _lines.at(_cursor.y).set(_cursor.x, _current); } break;
		case 0x7F: break; // ignore
		default:
		{
			CharInfo c;
			c.ch = i;
			c.attrib = _current.attrib;
			_lines.at(_cursor.y).set(_cursor.x, c);
			_cursor.x++;
			if (_cursor.x >= _size.x) { _cursor.y++; _cursor.x = 0; }
			if (_cursor.y >= _size.y) { scroll(1); _cursor.y = _size.y - 1; }
		}
		break;
		}

	}
};

namespace con {
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


// all from http://www.codeproject.com/Articles/1053/Using-an-output-stream-for-debugging
// https://cdot.senecacollege.ca/software/nled/nled_2_52_src/qkdisp.c
// alot of helper functions up there

template<class CharT, class TraitsT = std::char_traits<CharT> >
class basic_debugbuf : public std::basic_streambuf<CharT, TraitsT> {
public:
	std::array<char, 512> _buffer;
	std::basic_string<CharT, TraitsT> _str;
	bool write_console;
	bool write_visualstudio;
	int _lastc;
	void output_visualstudio(const std::basic_string<char>& str) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::basic_stringstream<CharT, TraitsT> ss;
		ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]:" << str << std::endl;
		::OutputDebugStringA(ss.str().c_str());
	}
	void output_visualstudio(const std::basic_string<wchar_t>& str) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::basic_stringstream<wchar_t> ss;
		ss << "[" << std::put_time(std::localtime(&in_time_t), L"%Y-%m-%d %X") << "]:" << str << std::endl;
		::OutputDebugStringA(ss.str().c_str());
	}
	void output_console(const char* str, size_t len) {
		DWORD dummy;
		::WriteConsoleA(console::hConsole, str, len, &dummy, NULL);
	}
	void output_console(const wchar_t* str, size_t len) {
		DWORD dummy;
		::WriteConsoleW(console::hConsole, str, len, &dummy, NULL);
	}
	int sync() override
	{
		std::ptrdiff_t n = pptr() - pbase();
		_str.reserve(n);
		if (write_visualstudio) {
			for (auto ptr = pbase(); ptr != pptr(); ptr++) {
				CharT ch = *ptr;
				if (ch == '\r' || ch == '\n') {
					if (_lastc != ch && (_lastc == '\r' || _lastc == '\n')) {
						ch = 0;
					}
					else {
						output_visualstudio(_str);
						_str.clear();
					}
				}
				else _str.push_back(ch);
				_lastc = ch;
			}
		}
		if (write_console) output_console(pbase(), n);
		pbump(-n);
		return 0;
	}
public:
	basic_debugbuf(bool write_console, bool write_visualstudio) : write_visualstudio(write_visualstudio), write_console(write_console), std::streambuf(), _lastc(0) {
		_str.reserve(512);
		char *base = _buffer.data();
		setp(base, base + _buffer.size() - 1); // -1 to make overflow() easier
	}
	
};


basic_debugbuf<char> s_cerr_debug_buffer(false, true);
basic_debugbuf<char> s_cout_debug_buffer(true, false);

namespace  {
	void init_cerr() {
	//	s_cerr_debug_buffer.setOldBuffer(std::cerr.rdbuf());
		std::cerr.rdbuf(&s_cerr_debug_buffer);
		std::cerr << "cerr redirected" << std::endl;
	}
	void init_cout() {
	//	s_cout_debug_buffer.setOldBuffer(std::cout.rdbuf());
		std::cout.rdbuf(&s_cout_debug_buffer);
		std::cout << "cout redirected" << std::endl;
	}
	void error(const std::string& message) {

	}
};