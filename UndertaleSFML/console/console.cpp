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
	COORD qd_coord;           /* current screen position */
	short qd_normattr = 0x07, /* normal, ...      */
		qd_boldattr = 0x0f, /* ...bold, and...  */
		qd_revattr = 0x70, /* ...reverse video */
		qd_saveattr = 0x07, /* save original attribute */
		qd_clrattr = 0x07; /* used to clear to end of line */
	short qd_numcols = 0,
		qd_numrows = 0,
		qd_drawtop = -1,    /* line to draw (-1: none, qd_numrows: all) */
		qd_drawcol = -1,    /* first column to redraw if one line only */
		qd_drawbot = 0;     /* draw status (bottom) line? */
	HANDLE qd_out, qd_back,   /* output and backbuffer consoles */
		qd_orig;           /* original console (to restore at end) */
	std::vector<CHAR_INFO> qd_buff;
	/* assuming we are drawing on line "row", set redrawing flags accordingly */
	void touchline(short row, short col)
	{
		if (row == -1) {                /* request for whole screen */
			qd_drawtop = qd_numrows;    /* ..the whole top section */
			qd_drawcol = -1;
			qd_drawbot = 1;             /*.. and the status line */
		}
		else if (row == qd_numrows - 1) /* is the status line */
			qd_drawbot = 1;
		else if (qd_drawtop == -1) {    /* is the first time drawing in the top */
			qd_drawtop = row;           /* ..just redraw this line */
			qd_drawcol = col;
		}
		else if (qd_drawtop != row) {   /* at least one other line in the top */
			qd_drawtop = qd_numrows;    /* ..so do the whole top */
			qd_drawcol = -1;
		}
		else if (col < qd_drawcol)      /* same line as before but left of prev */
			qd_drawcol = -1;
	}
	void qd_clearline(int row)
	{
		DWORD junk;
		qd_coord.X = 0;
		qd_coord.Y = row;
		FillConsoleOutputCharacter(qd_back, ' ', qd_numcols, qd_coord, &junk);
		FillConsoleOutputAttribute(qd_back, qd_normattr, qd_numcols, qd_coord, &junk);
		SetConsoleCursorPosition(qd_back, qd_coord);
		touchline(-1, -1);
	}
	void qd_clear(void)
	{
		DWORD junk;
		qd_coord.X = qd_coord.Y = 0;
		FillConsoleOutputCharacter(qd_back, ' ', qd_numrows * qd_numcols,
			qd_coord, &junk);
		FillConsoleOutputAttribute(qd_back, qd_normattr, qd_numrows * qd_numcols,
			qd_coord, &junk);
		SetConsoleCursorPosition(qd_back, qd_coord);
		touchline(-1, -1);
	}
	void qd_color(int color)
	{
		qd_normattr = color;
		qd_boldattr = color | 0x08;
		qd_revattr = ((color & 0x07) << 4) | ((color & 0x070) >> 4);
		qd_clrattr = qd_normattr;
		SetConsoleTextAttribute(qd_out, qd_normattr);
		SetConsoleTextAttribute(qd_back, qd_normattr);
	}

	int qd_cols(void)
	{
		return qd_numcols;
	}

	void qd_eeol(void)
	{
		DWORD junk;
		FillConsoleOutputCharacter(qd_back, ' ', qd_numcols - qd_coord.X,
			qd_coord, &junk);
		FillConsoleOutputAttribute(qd_back, qd_clrattr, qd_numcols - qd_coord.X,
			qd_coord, &junk);
		touchline(qd_coord.Y, qd_coord.X);
	}

	void qd_fixterm(void)
	{
		COORD c, orig;
		SMALL_RECT r;
		CONSOLE_SCREEN_BUFFER_INFO scr;

		/* copy screen into backbuffer */
		c.X = qd_numcols;
		c.Y = qd_numrows;
		orig.X = r.Left = 0;
		orig.Y = r.Top = 0;
		r.Right = qd_numcols - 1;
		r.Bottom = qd_numrows - 1;
		PERR(ReadConsoleOutput(qd_out, qd_buff.data(), c, orig, &r), "ReadConsoleOutput");
		PERR(WriteConsoleOutput(qd_back, qd_buff.data(), c, orig, &r), "WriteConsoleOutput");

		GetConsoleScreenBufferInfo(qd_out, &scr);
		qd_coord = scr.dwCursorPosition;
		SetConsoleCursorPosition(qd_back, qd_coord);
	}

	void qd_flush(void)
	{
		COORD c, orig;
		SMALL_RECT r;

		/* cut data from backbuffer and place it on screen console */
		if (qd_drawtop == qd_numrows && qd_drawbot) {
			c.X = qd_numcols;
			c.Y = qd_numrows;
			orig.X = orig.Y = 0;
			r.Left = 0;
			r.Right = qd_numcols - 1;
			r.Top = 0;
			r.Bottom = qd_numrows - 1;
			ReadConsoleOutput(qd_back, qd_buff.data(), c, orig, &r);
			WriteConsoleOutput(qd_out, qd_buff.data(), c, orig, &r);
		}
		else {
			if (qd_drawbot) {
				c.X = qd_numcols;
				c.Y = qd_numrows;
				orig.X = r.Left = 0;
				orig.Y = r.Top = qd_numrows - 1;
				r.Right = qd_numcols - 1;
				r.Bottom = qd_numrows - 1;
				ReadConsoleOutput(qd_back, qd_buff.data(), c, orig, &r);
				WriteConsoleOutput(qd_out, qd_buff.data(), c, orig, &r);
			}
			if (qd_drawtop == qd_numrows) {
				c.X = qd_numcols;
				c.Y = qd_numrows;
				orig.X = r.Left = 0;
				orig.Y = r.Top = 0;
				r.Right = qd_numcols - 1;
				r.Bottom = qd_numrows - 2;
				ReadConsoleOutput(qd_back, qd_buff.data(), c, orig, &r);
				WriteConsoleOutput(qd_out, qd_buff.data(), c, orig, &r);
			}
			else if (qd_drawtop != -1) {
				c.X = qd_numcols;
				c.Y = qd_numrows;
				orig.X = r.Left = (qd_drawcol == -1 ? 0 : qd_drawcol);
				orig.Y = r.Top = qd_drawtop;
				r.Right = qd_numcols - 1;
				r.Bottom = qd_drawtop;
				ReadConsoleOutput(qd_back, qd_buff.data(), c, orig, &r);
				WriteConsoleOutput(qd_out, qd_buff.data(), c, orig, &r);
			}
		}
		qd_drawtop = qd_drawcol = -1;
		qd_drawbot = 0;

		SetConsoleCursorPosition(qd_out, qd_coord);
	}
	int qd_getch(void)
	{
		int c = 0;
		//c = _getch();
		if (c == 224 || c == 0) {
			//	c = _getch() | 0xFF00;
			if (c == 0xFF86)
				c = 0xFF84; /* ctrl PgUp has changed */
		}
		return c;
	}
	void qd_move(int row, int col)
	{
		qd_coord.X = col;
		qd_coord.Y = row;
		SetConsoleCursorPosition(qd_back, qd_coord);
	}
	void qd_close(void)
	{
		DWORD junk;
		extern void qd_flush(void);
		touchline(-1, -1);
		qd_flush();

		/*  old Windows code that moved down one line before exiting:

		qd_coord.X = 0;
		qd_coord.Y = qd_numrows - 1;
		FillConsoleOutputAttribute(qd_out, qd_saveattr, qd_numcols, qd_coord,
		&junk);
		SetConsoleTextAttribute(qd_out, qd_saveattr);
		SetConsoleCursorPosition(qd_out, qd_coord);
		*/

		SetConsoleActiveScreenBuffer(qd_orig);
		CloseHandle(qd_out);
		CloseHandle(qd_back);
		qd_buff.clear();
	}
	void qd_open(void)
	{
		DWORD mode;
		CONSOLE_SCREEN_BUFFER_INFO scr;
		CONSOLE_CURSOR_INFO cursinfo;
		COORD c, orig;
		SMALL_RECT r;

		qd_orig = GetStdHandle(STD_OUTPUT_HANDLE);
		qd_out = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		qd_back = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		if (qd_orig && qd_out && qd_back) {
			GetConsoleCursorInfo(qd_orig, &cursinfo);
			GetConsoleScreenBufferInfo(qd_orig, &scr);
			c.Y = qd_numrows = scr.srWindow.Bottom - scr.srWindow.Top + 1;
			c.X = qd_numcols = scr.srWindow.Right - scr.srWindow.Left + 1;

			SetConsoleScreenBufferSize(qd_out, c);
			SetConsoleScreenBufferSize(qd_back, c);

			SetConsoleActiveScreenBuffer(qd_out);

			SetConsoleTitle("NLED");
			SetConsoleMode(qd_out, 0);
			SetConsoleMode(qd_back, 0);
			cursinfo.bVisible = FALSE;
			SetConsoleCursorInfo(qd_back, &cursinfo);

			qd_saveattr = scr.wAttributes;
			qd_buff.resize(qd_numrows * qd_numcols);
			qd_fixterm();

			/* In fullscreen mode only, fixterm turns off qd_out's */
			/* cursor, so this MUST be done down here !?!?!        */
			cursinfo.bVisible = TRUE;
			SetConsoleCursorInfo(qd_out, &cursinfo);
		}
	}
	void qd_puts(int length, const char *string)
	{
		DWORD junk;
		int i;
		char c;
		for (i = 0; i < length; i++) {
			switch (c = string[i]) {
			case '\b': case '\r': case '\n': break; // skip
			default:
				WriteConsole(qd_back, &c, 1, &junk, NULL);
				break;
			}
		}
		touchline(qd_coord.Y, qd_coord.X);
		qd_coord.X += length;
	}

	void qd_resetterm(void)
	{
		qd_flush();
		SetConsoleTextAttribute(qd_out, qd_normattr);
		SetConsoleCursorPosition(qd_out, qd_coord);
	}
	int qd_rows(void)
	{
		return qd_numrows;
	}
	void qd_scrollup(int startrow, int endrow)
	{
		SMALL_RECT tot, scr;
		COORD dest;
		CHAR_INFO ch;
		tot.Left = 0;
		tot.Right = qd_numcols - 1;
		tot.Top = startrow;
		tot.Bottom = endrow;
		scr.Left = 0;
		scr.Right = qd_numcols - 1;
		scr.Top = startrow + 1;
		scr.Bottom = endrow;
		dest.X = 0;
		dest.Y = startrow;
		ch.Char.AsciiChar = ' ';
		ch.Attributes = qd_normattr;
		ScrollConsoleScreenBuffer(qd_back, &scr, &tot, dest, &ch);
		touchline(startrow, 0);
		touchline(endrow, 0);
	}

	void qd_scrolldown(int startrow, int endrow)
	{
		SMALL_RECT tot, scr;
		COORD dest;
		CHAR_INFO ch;
		tot.Left = 0;
		tot.Right = qd_numcols - 1;
		tot.Top = startrow;
		tot.Bottom = endrow;
		scr.Left = 0;
		scr.Right = qd_numcols - 1;
		scr.Top = startrow;
		scr.Bottom = endrow - 1;
		dest.X = 0;
		dest.Y = startrow + 1;
		ch.Char.AsciiChar = ' ';
		ch.Attributes = qd_normattr;
		ScrollConsoleScreenBuffer(qd_back, &scr, &tot, dest, &ch);
		touchline(startrow, 0);
		touchline(endrow, 0);
	}

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
		qd_open();
		//	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
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
		int sync() override
		{
			std::ptrdiff_t n = pptr() - pbase();
			DWORD dummy;
			::WriteConsoleA(con_buffer, pbase(), n, &dummy, NULL);

			
			refresh();
			return 0;
		}
		HANDLE con_buffer;
		COORD pos;
		COORD size;
		std::vector<CHAR_INFO> screen;
		CONSOLE_CURSOR_INFO cursinfo;
		CONSOLE_SCREEN_BUFFER_INFO scrinfo;
		void refresh() {
			COORD c, orig;
			SMALL_RECT r;
			CONSOLE_SCREEN_BUFFER_INFO scr;
			GetConsoleScreenBufferInfo(qd_out, &scr);
			/* copy screen into backbuffer */
			c.X = qd_numcols;
			c.Y = qd_numrows;
			orig.X = r.Left = 0;
			orig.Y = r.Top = 0;
			r.Right = size.X - 1;
			r.Bottom = size.Y - 1;
			PERR(ReadConsoleOutput(con_buffer, screen.data(), size, { 0,0 }, &r), "ReadConsoleOutput");
			PERR(WriteConsoleOutput(qd_back, screen.data(), size, pos, &r), "WriteConsoleOutput");

			touchline(-1, -1);
			qd_flush();
		}
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
class basic_debugbuf : public std::streambuf {
protected:
	std::array<char, 1024> _buffer;
	std::streambuf * _oldBuffer;
	std::string _outBuffer;
	bool _endOfLineSeen = false;
	int _lastc = 0;
	COORD _cursor;
	void output() {
		if (!is_string_empty_or_whitespace(_outBuffer)) { // check for empty sync
			//console::qd_move(_cursor.Y, _cursor.X);
			if (_cursor.Y >= console::qd_rows()-1) console::qd_scrollup(15, _cursor.Y - 2);
			else _cursor.Y++;
			console::qd_clearline(_cursor.Y-1);
			//console::qd_clear();
			auto now = std::chrono::system_clock::now();
			auto in_time_t = std::chrono::system_clock::to_time_t(now);
			std::stringstream ss;
			ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]:" << _outBuffer << std::endl;
			//ss << "[]" << _outBuffer;//  << std::endl;
			
			console::qd_puts(ss.str().length(), ss.str().c_str());

			::OutputDebugStringA(ss.str().c_str());
			//if (_oldBuffer) _oldBuffer->sputn(ss.str().c_str(), ss.str().length());
			//PERR(GetConsoleScreenBufferInfo(console::hConsole, &info), "GetConsoleScreenBufferInfo");
			console::qd_flush();
			
		}
		_outBuffer.clear();
	}
	void add_buffer(int_type ch) {
		if (ch == '\n' || ch == '\r')
		{
			if (_lastc != ch && (_lastc == '\n' || _lastc == '\r')) {
				// skip, throw it away
				_lastc = 0;
				return;
			}
			output();
		}
		else if (ch == traits_type::eof()) {
			output();
		}
		else
			_outBuffer.push_back(_lastc = ch);
	}
	static bool is_string_empty_or_whitespace(const std::string& str) {
		if (str.length() > 0) for (char c : str) if (!isspace(c))return false;
		return true;
	}

	int sync() override
	{
		for (auto ptr = pbase(); ptr != pptr(); ptr++) add_buffer(*ptr);
		std::ptrdiff_t n = pptr() - pbase();
		pbump(-n);
		return 0;
	}

public:
	basic_debugbuf() : std::streambuf(), _oldBuffer(nullptr), _lastc(0) {
		char *base = _buffer.data();
		setp(base, base + _buffer.size() - 1); // -1 to make overflow() easier
		_cursor.X = 0; _cursor.Y = 15;
	}
	void setOldBuffer(std::streambuf* old_buffer) { _oldBuffer = old_buffer; _cursor.X = 0; _cursor.Y = 15; };
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