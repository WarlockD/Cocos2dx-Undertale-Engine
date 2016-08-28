#include <windows.h>
#ifdef COORD
#undef COORD
#endif
#ifdef SMALL_RECT
##undef SMALL_RECT
#endif
#include "console.h"

// all from http://www.codeproject.com/Articles/1053/Using-an-output-stream-for-debugging
class basic_debugbuf : public std::streambuf {
protected:
	std::array<char, 1024> _buffer;
	std::streambuf * _oldBuffer;
	int _lastc;
	int_type overflow(int_type ch) override
	{
		if (ch == '\n' || ch == '\r')
		{
			if (_lastc != ch && (_lastc == '\n' || _lastc == '\r')) {
				// skip, throw it away
				_lastc = 0;
				return 0;
			}
			*pptr() = '\n';
			pbump(1);
			sync();
		}
		else if (ch == traits_type::eof()) {
			*pptr() = '\n';
			pbump(1);
			sync();
			return traits_type::eof();
		}
		else {
			*pptr() = ch;
			pbump(1);
		}
		return 0;
	}
	static bool is_string_empty_or_whitespace(const std::string& str) {
		bool empty = true;
		if (str.length() > 0) {
			for (char c : str) if (!isspace(c)) { empty = false; break; }
		}
		return empty;
	}
	int sync() override
	{
		std::string str(pbase(), pptr());
		if (!is_string_empty_or_whitespace(str)) { // check for empty sync
			auto now = std::chrono::system_clock::now();
			auto in_time_t = std::chrono::system_clock::to_time_t(now);
			std::stringstream ss;
			ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]:" << str << std::endl;

			output_debug_string(ss.str().c_str());
			if (_oldBuffer) _oldBuffer->sputn(ss.str().c_str(), ss.str().length());
		}
		std::ptrdiff_t n = pptr() - pbase();
		pbump(-n); // clear the buffer
		return 0;
	}

	void output_debug_string(const char *text) {
		::OutputDebugStringA(text);
	}

public:
	virtual ~basic_debugbuf()
	{
		sync();
	}
	basic_debugbuf() : std::streambuf(), _oldBuffer(nullptr), _lastc(0) {
		char *base = _buffer.data();
		setp(base, base + _buffer.size() - 1); // -1 to make overflow() easier
	}
	void setOldBuffer(std::streambuf* old_buffer) { _oldBuffer = old_buffer; }
};




template<class CharT, class TraitsT = std::char_traits<CharT> >
class basic_dostream : public std::basic_ostream<CharT, TraitsT>
{
public:
	basic_dostream() : std::basic_ostream<CharT, TraitsT>
		(new basic_debugbuf<CharT, TraitsT>()) {}
	~basic_dostream()
	{
		//std::stringstream ss;
		delete rdbuf();
	}

	void setOldBuffer(std::basic_stringbuf<CharT, TraitsT>* old_buffer) {
		auto debug_stream = dynamic_cast<basic_debugbuf<CharT, TraitsT>*>(rdbuf());
		debug_stream->setOldBuffer(old_buffer);
	}
};

typedef basic_dostream<char>    dostream;
typedef basic_dostream<wchar_t> wdostream;

basic_debugbuf s_cerr_debug_buffer;
basic_debugbuf s_cout_debug_buffer;

namespace logging {
	void init_cerr() {
		s_cerr_debug_buffer.setOldBuffer(std::cerr.rdbuf());
		std::cerr.rdbuf(&s_cerr_debug_buffer);
		std::cerr << "cerr redirected" << std::endl;
	}
	void init_cout() {
		s_cout_debug_buffer.setOldBuffer(std::cout.rdbuf());
		std::cout.rdbuf(&s_cout_debug_buffer);
		std::cout << "cout redirected" << std::endl;
	}
	bool init() {
		return true;
	}
	void error(const std::string& message) {

	}
};
#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %d from %s on line %d\n", __FILE__, GetLastError(), api, __LINE__);}
namespace console {
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
			attr |= static_cast<WORD>(color)<<4;

			::SetConsoleTextAttribute(console, attr);
		}

		void _setcolors(HANDLE const console, console::Color const fg,  console::Color const bg)
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
		ScreenInfo setScreenInfo(HANDLE const console) {
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

		logging::init_cerr();
		logging::init_cout();
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		PERR(hConsole, "GetStdHandle");
		PERR(GetConsoleScreenBufferInfo(hConsole, &console_info), "GetConsoleScreenBufferInfo");
		console_window = console_info.srWindow;
		console_buffer_size = { console_window.Right - console_window.Left , console_window.Bottom - console_window.Top };
		console_buffer.resize(console_buffer_size.X * console_buffer_size.Y, { ' ', console_info.wAttributes });
	}
	/* Standard error macro for reporting API errors */
	void gotoxy(int x, int y) {
		_details::_setpos(hConsole, Point(x,y));
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
	void cls()
	{
		if (hConsole == NULL) return;
		COORD coordScreen = { 0, 0 };    /* here's where we'll home the
										 cursor */
		BOOL bSuccess;
		DWORD cCharsWritten;
		CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
		DWORD dwConSize;                 /* number of character cells in
										 the current buffer */

										 /* get the number of character cells in the current buffer */

		bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
		PERR(bSuccess, "GetConsoleScreenBufferInfo");
		dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

		/* fill the entire screen with blanks */

		bSuccess = FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
			dwConSize, coordScreen, &cCharsWritten);
		PERR(bSuccess, "FillConsoleOutputCharacter");

		/* get the current text attribute */

		bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
		PERR(bSuccess, "ConsoleScreenBufferInfo");

		/* now set the buffer's attributes accordingly */

		bSuccess = FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
			dwConSize, coordScreen, &cCharsWritten);
		PERR(bSuccess, "FillConsoleOutputAttribute");

		/* put the cursor at (0, 0) */

		bSuccess = SetConsoleCursorPosition(hConsole, coordScreen);
		PERR(bSuccess, "SetConsoleCursorPosition");
		return;
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
	std::ostream& vt100() {
		return std::cout;
	} // stream for vt100 emulation on console, only simple escape codes are supported however
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