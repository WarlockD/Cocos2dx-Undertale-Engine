#include "vt100.h"
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <array>
#include <cassert>
#include <queue>
#include <algorithm>
#include <Windows.h>
# define A(x) ((wchar_t)x)
static const wchar_t acs_map[128] =
{
	A(0), A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8), A(9), A(10),
	A(11), A(12), A(13), A(14), A(15), A(16), A(17), A(18), A(19),
	A(20), A(21), A(22), A(23), A(24), A(25), A(26), A(27), A(28),
	A(29), A(30), A(31), ' ', '!', '"', '#', '$', '%', '&', '\'', '(',
	')', '*',
	0x2192, 0x2190, 0x2191, 0x2193,
	'/',
	0x2588,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=',
	'>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	0x2666, 0x2592,
	'b', 'c', 'd', 'e',
	0x00b0, 0x00b1, 0x2591, 0x00a4, 0x2518, 0x2510, 0x250c, 0x2514,
	0x253c, 0x23ba, 0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524,
	0x2534, 0x252c, 0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3,
	0x00b7,
	A(127)
};

namespace std {
	template<class T, class Compare>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
	{
		return assert(!comp(hi, lo)),comp(v, lo) ? lo : comp(hi, v) ? hi : v;
	}
	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi)
	{
		return clamp(v, lo, hi, std::less<>());
	}
};
static std::vector<CHAR_INFO> s_back_buffer;
static std::vector<CHAR_INFO> s_blink_buffer;
static HWND hConWnd = 0;                   // Console window handle
static HANDLE hConOut = INVALID_HANDLE_VALUE;                 // handle to CONOUT$
#define ESC     '\x1B'          // ESCape character
#define LF      '\x0A'          // Line Feed

#define FOREGROUND_BLACK 0
#define FOREGROUND_WHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define BACKGROUND_BLACK 0
#define BACKGROUND_WHITE BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE
WORD foregroundcolor[16] = {
	FOREGROUND_BLACK,                                       // black foreground
	FOREGROUND_RED,                                         // red foreground
	FOREGROUND_GREEN,                                       // green foreground
	FOREGROUND_RED | FOREGROUND_GREEN,                        // yellow foreground
	FOREGROUND_BLUE,                                        // blue foreground
	FOREGROUND_BLUE | FOREGROUND_RED,                         // magenta foreground
	FOREGROUND_BLUE | FOREGROUND_GREEN,                       // cyan foreground
	FOREGROUND_WHITE,                                       // white foreground
	FOREGROUND_BLACK | FOREGROUND_INTENSITY,                  // black foreground bright
	FOREGROUND_RED | FOREGROUND_INTENSITY,                    // red foreground bright
	FOREGROUND_GREEN | FOREGROUND_INTENSITY,                  // green foreground bright
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,   // yellow foreground bright
	FOREGROUND_BLUE | FOREGROUND_INTENSITY ,                  // blue foreground bright
	FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,    // magenta foreground bright
	FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,  // cyan foreground bright
	FOREGROUND_WHITE | FOREGROUND_INTENSITY                   // gray foreground bright
};

WORD backgroundcolor[16] = {
	BACKGROUND_BLACK,                                       // black background
	BACKGROUND_RED,                                         // red background
	BACKGROUND_GREEN,                                       // green background
	BACKGROUND_RED | BACKGROUND_GREEN,                        // yellow background
	BACKGROUND_BLUE,                                        // blue background
	BACKGROUND_BLUE | BACKGROUND_RED,                         // magenta background
	BACKGROUND_BLUE | BACKGROUND_GREEN,                       // cyan background
	BACKGROUND_WHITE,                                       // white background
	BACKGROUND_BLACK | BACKGROUND_INTENSITY,                  // black background bright
	BACKGROUND_RED | BACKGROUND_INTENSITY,                    // red background bright
	BACKGROUND_GREEN | BACKGROUND_INTENSITY,                  // green background bright
	BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,   // yellow background bright
	BACKGROUND_BLUE | BACKGROUND_INTENSITY,                   // blue background bright
	BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY,    // magenta background bright
	BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY,  // cyan background bright
	BACKGROUND_WHITE | BACKGROUND_INTENSITY                   // white background bright
};

// Table to convert the color order of the console in the ANSI order.
WORD conversion[16] = { 0, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15 };

// screen attributes


// parser is built from the state diagram from here
// http://vt100.net/emu/dec_ansi_parser
// used the codes from here, but I could get more in here..humm
// https://en.wikipedia.org/wiki/ANSI_escape_code
class Terminal {
protected:
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
	std::string _escape_parms;
	State _state = State::ground;
	std::array<int,16> _parms;
	int _current_parm = 0;
	virtual void csi_dispatch(char c, const std::array<int, 16>& parms, const std::string& text)  {}
	virtual void flush() = 0;
	virtual void pushch(int c) = 0;
	virtual void escape_dispatch(char c, const std::array<int, 16>& parms, const std::string& text) {}
	void clear() {
		_escape_parms.clear();
		flush();
		_parms.fill(0);// 30 - 39) and the semicolon(code 3B
		_current_parm = 0;
	}
	void escape_dispatch(char c) {
		flush();
		escape_dispatch(c, _parms,_escape_parms);
		clear();
	}
	void csi_dispatch(char c) {
		flush();
		csi_dispatch(c, _parms, _escape_parms);
		clear();
	}
	inline bool is_csi_state() const { return _state >= State::csi_entry && _state <= State::count_csi; }
	const int& current_parm() const { return _parms[_current_parm]; }
	int& current_parm() { return _parms[_current_parm]; }
	bool parm(char c) {
		if (c >= 0x30 && c <= 0x39 || c == 0x3B) {
			_escape_parms.push_back(c);
			if (c == 0x3B)
				_current_parm++;
			else
				_parms[_current_parm] = (_parms[_current_parm] * 10) + (c-0x30);
			return true;
		}
		return false;
	}
	void collect(char c) { _escape_parms.push_back(c); _parms[_current_parm++] = c; }
	bool execute(char c) {
		if ((c >= 0x00 && c <= 0x17) || c == 0x19 || (c >= 0x1C && c <= 0x1F)) return true; // execute, happens in any state
		return false;
	}
	
	
public:
	void putch(char c) {
		if (_state == State::csi_ignore) {
			if (!execute(c) && c != 0x7f && !(c >= 0x40 && c <= 0x7e))_state = State::ground;
			else return;
		}
		switch (c) {
		case 0x1B: _state = State::escape; 
			clear(); return;// escape
														   //	case 0x9D: _state = State::osc_string; return;// escape
		}
		if (is_csi_state()) {
			if (c == 0x7f || execute(c)) return;
			if (c >= 0x40 && c <= 0x7e) _state = State::csi_dispatch;
		}
		while (true) {
			switch (_state) {
			case State::ground: pushch(c); break;
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
class VT100 : public Terminal {
protected:
	static constexpr COORD ZCOORD = { 0,0 };
	bool bold = false;
	bool underline = false;
	bool rvideo = false;
	bool concealed = false;
	HANDLE _conOut;
	WORD foreground = FOREGROUND_WHITE;
	WORD background = BACKGROUND_BLACK;
	WORD foreground_default = FOREGROUND_WHITE;
	WORD background_default = BACKGROUND_BLACK;
	COORD SavePos;
	CONSOLE_SCREEN_BUFFER_INFO _original;
	CONSOLE_SCREEN_BUFFER_INFO Info;
	std::vector<wchar_t> _buffer;
	virtual void flush() override {
		if (_buffer.size() > 0) {
			DWORD nWritten;
			::WriteConsoleW(_conOut, _buffer.data(), _buffer.size(), &nWritten, NULL);
			_buffer.clear();
		}
	}
	int graphic_set = 0;

	virtual void pushch(int c) override {
		switch (c) {
		case 0x00: return;// ignore
		case 0x05: assert(false); break; // enquery
		case 0x07: break; // bell, works in console
		case 0x08: break; // backspace, works in console
		case 0x09: break; // tab, humm have to fix
		case 0x0A: break; // line feed, works in console
		case 0x0B: c = 0x10; break; // VT, prossed as lf
		case 0x0C:  
			clear_from_pos(ZCOORD, Info.dwSize.X*Info.dwSize.Y);
			home_cursor(); // FF clear screen?
			// c = 0x10; 
			return; // FF, prossed as lf
		case 0x0D: break; // should work? moves to left margin
		case 0x0E: graphic_set = 1; return; // shift out? change charater set to GL  G1 is sect SCS
		case 0x0F: graphic_set = 0; return; // shift in? change charater set to GL, G0 is SCS
		case 0x10: assert(false); break; // Also referred to as XON. If XOFF support is enabled, DC1 clears DC3 (XOFF), causing the terminal to continue transmitting characters (keyboard unlocks) unless KAM mode is currently set.
		case 0x13: assert(false); break; // Also referred to as XOFF. If XOFF support is enabled, DC3 causes the terminal to stop transmitting characters until a DC1 control character is received.
		case 0x18: assert(false); break; // If received during an escape or control sequence, terminates and cancels the sequence. No error character is displayed. If received during a device control string, the DCS is terminated and no error character is displayed.
		case 0x1A: assert(false); break; // 	If received during escape or control sequence, terminates and cancels the sequence. Causes a reverse question mark to be displayed. If received during a device control sequence, the DCS is terminated and reverse question mark is displayed.
		case 0x1B: assert(false); break; // ESC, should never get here
		case 0x7F: break; // we ignore these should never get in
		}
		if (graphic_set == 1) c = acs_map[c];
		_buffer.push_back(c);
	}

	const CHAR_INFO blank_char() const { return{ (USHORT)' ', Info.wAttributes }; }
	const COORD line_start() const { return{ 0, Info.dwCursorPosition.Y }; }
	const COORD& cursor() const { return Info.dwCursorPosition; }
	void cursor(COORD pos)  { 
		pos.X = std::clamp(pos.X, short(0), short(Info.dwSize.X - 1));
		pos.Y = std::clamp(pos.Y, short(0), short(Info.dwSize.Y - 1));
		::SetConsoleCursorPosition(_conOut, pos);
		Info.dwCursorPosition = pos;
	}
	void home_cursor() {
		::SetConsoleCursorPosition(_conOut, ZCOORD);
		Info.dwCursorPosition = ZCOORD;
	}
	void reset_attributes() {
		foreground = foreground_default;
		background = background_default;
		bold = false;
		underline = false;
		rvideo = false;
		concealed = false;
	}
	void clear_from_pos(const COORD& pos, const DWORD& len, bool keep_attrib=false) {
		DWORD NumberOfCharsWritten;
		::FillConsoleOutputCharacter(_conOut, ' ', len, pos, &NumberOfCharsWritten);
		if(keep_attrib) ::FillConsoleOutputAttribute(_conOut, Info.wAttributes, len, pos, &NumberOfCharsWritten);
	}
	void clear_screen(int i) {
		switch (i) {
		case 0: // ESC[J || ESC[0J erase from cursor to end of display
			clear_from_pos(Info.dwCursorPosition, Info.dwCursorPosition.Y*Info.dwSize.X + Info.dwCursorPosition.X + 1); 
			break;
		case 1: // ESC[1J erase from start to cursor.
			clear_from_pos(ZCOORD, (Info.dwSize.Y - Info.dwCursorPosition.Y - 1) *Info.dwSize.X + Info.dwSize.X - Info.dwCursorPosition.X - 1);
			break;
		case 2: //ESC[2J Clear screen and home cursor
			clear_from_pos(ZCOORD, Info.dwSize.X*Info.dwSize.Y);
			::SetConsoleCursorPosition(_conOut, ZCOORD);
			break;
		}
	}
	
	void ScrollConsole(const SMALL_RECT& rect, const COORD& pos) {
		CHAR_INFO blank = { (USHORT)' ', Info.wAttributes };
		::ScrollConsoleScreenBuffer(_conOut, &rect, NULL, pos, &blank);
	}
	/*

	void insert_line(int lineno,int count) {
		if (count == 0) count = 1; // ESC[L == ESC[1L
		ScrollConsole({ 0, lineno, Info.dwSize.X - short(1), Info.dwSize.Y - short(1) }, { 0, lineno + count });
		clear_from_pos({ 0, lineno }, Info.dwSize.X*count);
	}
	void delete_line(int lineno, int count) {
		if (count == 0) count = 1; // ESC[L == ESC[1L
		if (lineno > (lineno - Info.dwCursorPosition.Y)) lineno = Info.dwSize.Y - lineno;
		ScrollConsole({ 0, lineno, Info.dwSize.X - short(1), Info.dwSize.Y - short(1) }, { 0, lineno });
		clear_from_pos({ 0, lineno -count }, Info.dwSize.X*count);
	}
	*/
	virtual void csi_dispatch(char c, const std::array<int, 16>& parms, const std::string& text)  override {
		WORD attribut;
		::GetConsoleScreenBufferInfo(_conOut, &Info);
		static const char* modes[] = {
			"Cursor key ",
			"ANSI // VT52 ",
			"Column mode",
			"Scrolling",
			"Screen mode",
			"Origin mode",
			"Wraparound",
			"Autorepeat",
			"Interface"
		};
		// http://umich.edu/~archive/apple2/misc/programmers/vt100.codes.txt helpful
		switch (c) {
			//Line feed / new   New line        ESC[20h        Line feed       ESC[20l
			//	Cursor key      Application     ESC[? 1h        Cursor          ESC[? 1l
			//	ANSI / VT52       ANSI            n / a             VT52            ESC[? 2l
			//	Column mode     132 col         ESC[? 3h        80 col          ESC[? 3l
			//	Scrolling       Smooth          ESC[? 4h        Jump            ESC[? 4l
			//	Screen mode     Reverse         ESC[? 5h        Normal          ESC[? 5l
			//	Origin mode     Relative        ESC[? 6h        Absolute        ESC[? 6l
			//	Wraparound      On              ESC[? 7h        Off             ESC[? 7l
			//	Autorepeat      On              ESC[? 8h        Off             ESC[? 8l
			//	Interface       On              ESC[? 9h        Off             ESC[? 9l
		case 'l': // resset mode
			if (parms[0] == 0x20)
				std::cerr << "Mode Reset : " << modes[0] << std::endl;
			else if(parms[0] =='?') {
				std::cerr << "Mode Reset : " << modes[parms[1]] << std::endl;
			}
			else {
				assert(false);
			}
			break;
		case 'h': // set mode
			if (parms[0] == 0x20)
				std::cerr << "Mode Set : " << modes[0] << std::endl;
			else if (parms[0] == '?') {
				std::cerr << "Mode Set : " << modes[parms[1]] << std::endl;
			}
			else {
				assert(false);
			}
			break;
		case 'm':
			if(parms[0] == 0)reset_attributes();
			else {
				for (int i = 0; parms[i] != 0; i++) {
					short value = parms[i];
					switch (value) {
					case 0:reset_attributes(); break;
					case 1: bold = true; break;
					case 21: bold = false; break;
					case 4: underline = true; break;
					case 24: underline = false; break;
					case 7: rvideo = true; break;
					case 27: rvideo = false; break;
					case 8: concealed = true; break;
					case 28: concealed = false; break;
					default:
						if (value >= 30  &&  value <= 37) foreground = parms[i] - 30;
						else if (value >= 40 && value <= 47) background = parms[i] - 40;
						break;
					}
				}
			}			
			if (rvideo) attribut = foregroundcolor[background] | backgroundcolor[foreground];
			else attribut = foregroundcolor[foreground] | backgroundcolor[background];
			if (bold) attribut |= FOREGROUND_INTENSITY;
			if (underline) attribut |= BACKGROUND_INTENSITY;
			//	DEBUGSTR("set console color to = 0x%.8x", attribut);
			SetConsoleTextAttribute(_conOut, attribut);
			//Info.wAttributes = attribut;
			break;
		case 'J':
			switch (parms[0]) {
			case 0: // ESC[J || ESC[0J erase from cursor to end of display
				clear_from_pos(Info.dwCursorPosition, Info.dwCursorPosition.Y*Info.dwSize.X + Info.dwCursorPosition.X + 1);
				break;
			case 1: // ESC[1J erase from start to cursor.
				clear_from_pos(ZCOORD, (Info.dwSize.Y - Info.dwCursorPosition.Y - 1) *Info.dwSize.X + Info.dwSize.X - Info.dwCursorPosition.X - 1);
				break;
			case 2: //ESC[2J Clear screen and home cursor
				clear_from_pos(ZCOORD, Info.dwSize.X*Info.dwSize.Y);
				home_cursor();
				break;
			}
			break;
		case 'K':
			switch (parms[0]) {
			case 0: //  ESC[K || ESC[0K Clear to end of line
				clear_from_pos(Info.dwCursorPosition, Info.srWindow.Right - Info.dwCursorPosition.X + 1);
				break;
			case 1: // ESC[1K Clear from start of line to cursor
				clear_from_pos({ 0,Info.dwCursorPosition.Y }, Info.dwCursorPosition.X + 1);
				break;
			case 2:  // ESC[2K Clear whole line.
				clear_from_pos({ 0,Info.dwCursorPosition.Y }, Info.dwSize.X);
				break;
			}
			break;
		case 'L':  // ESC[L == ESC[1L
		{
			short count = parms[0];
			if (count == 0) count = 1;
			ScrollConsole({ 0, Info.dwCursorPosition.Y, Info.dwSize.X -short(1), Info.dwSize.Y - short(1) }, { 0, Info.dwCursorPosition.Y + count });
			clear_from_pos({ 0, Info.dwCursorPosition.Y }, Info.dwSize.X*count);
		}
		break;
		case 'M':// ESC[L == ESC[1L
		{
			short count = parms[0];
			if (count == 0) count = 1;
			if (count > (Info.dwSize.Y - Info.dwCursorPosition.Y)) count = Info.dwSize.Y - Info.dwCursorPosition.Y;
			ScrollConsole({ 0, count, Info.dwSize.X - 1, Info.dwSize.Y - 1 }, { 0, Info.dwCursorPosition.Y });
			clear_from_pos({ 0,  Info.dwSize.Y - count }, Info.dwSize.X*count);
		}
		break;
		case 'P': // ESC[#P Delete # characters.
		{
			short count = parms[0];
			if (count == 0) count = 1;// ESC[P == ESC[1P
			if ((Info.dwCursorPosition.X + count) > (Info.dwSize.X - 1)) count = Info.dwSize.X - Info.dwCursorPosition.X;
			ScrollConsole({ Info.dwCursorPosition.X + count, Info.dwCursorPosition.Y, Info.dwSize.X - 1, Info.dwCursorPosition.Y }, Info.dwCursorPosition);
			clear_from_pos({ Info.dwSize.X - count,Info.dwCursorPosition.Y }, count, true);
		}
		break;
		case '@': // ESC[#@ Insert # blank characters.
		{
			short count = parms[0];
			if (count == 0) count = 1;   // ESC[@ == ESC[1@
			if (Info.dwCursorPosition.X + count > Info.dwSize.X - 1) count = Info.dwSize.X - Info.dwCursorPosition.X;
			ScrollConsole({ Info.dwCursorPosition.X, Info.dwCursorPosition.Y, Info.dwSize.X - 1 - count, Info.dwCursorPosition.Y }, { Info.dwCursorPosition.X + count, Info.dwCursorPosition.Y });
			clear_from_pos(Info.dwCursorPosition, count);
		}
		break;
		case 'A':  // ESC[#A Moves cursor up # lines
		{
			short count = parms[0];
			if (count == 0) count = 1;    // ESC[A == ESC[1A
			cursor({ Info.dwCursorPosition.X, Info.dwCursorPosition.Y - count });
		}
		break;
		case 'B':  // ESC[#B Moves cursor down # lines
		{
			short count = parms[0];
			if (count == 0) count = 1;    // ESC[B == ESC[1B
			cursor({ Info.dwCursorPosition.X, Info.dwCursorPosition.Y + count });
		}
		break;
		case 'C':   // ESC[#C Moves cursor forward # spaces
		{
			short count = parms[0];
			if (count == 0) count = 1;    // ESC[C == ESC[1C
			cursor({ Info.dwCursorPosition.X + count, Info.dwCursorPosition.Y });
		}
		break;
		case 'D':    // ESC[#D Moves cursor back # spaces
		{
			short count = parms[0];
			if (count == 0) count = 1;    // ESC[D == ESC[1D
			cursor({ Info.dwCursorPosition.X - count, Info.dwCursorPosition.Y });
		}
		break;
		case 'E':    // ESC[#E Moves cursor down # lines, column 1.
		{
			short count = parms[0];
			if (count == 0) count = 1;     // ESC[E == ESC[1E
			cursor({ 0,  Info.dwCursorPosition.Y + count });
		}
		break;
		case 'F':    // ESC[#F Moves cursor up # lines, column 1.
		{
			short count = parms[0];
			if (count == 0) count = 1;     // ESC[F == ESC[1F
			cursor({ 0,  Info.dwCursorPosition.Y - count });
		}
		break;
		case 'G':    // ESC[#G Moves cursor column # in current row.
		{
			short count = parms[0];
			if (count == 0) count = 1;     // ESC[G == ESC[1G
			cursor({ count - 1,  Info.dwCursorPosition.Y });
		}
		break;
		case 'q': // leds on or off
			if (parms[0])
				std::cerr << "DECLL L1 on";
			else
				std::cerr << "DECLL L1 off";
			break;
		case 'f':
		case 'H':                               // ESC[#;#H or ESC[#;#f Moves cursor to line #, column #
		{
			cursor({ short(parms[1] == 0 ? 0 : parms[1] - 1) ,  short(parms[0] == 0 ? 0 : parms[0] - 1) });
		}
		break;
		case 's':                               // ESC[s Saves cursor position for recall later
			SavePos = Info.dwCursorPosition;
			break;
		case 'u':                               // ESC[u Return to saved cursor position
			cursor(SavePos);
			return;
		default:
			std::cerr << "Unknown ESC command " << c << std::endl;
			assert(false);
			break;
		}
	}

	bool _dec52Mode = false;
	bool dec52(char c, const std::array<int, 16>& parms) {
		switch (c) {
		case 'A':  cursor({ Info.dwCursorPosition.X, Info.dwCursorPosition.Y - 1 }); break;
		case 'B':  cursor({ Info.dwCursorPosition.X, Info.dwCursorPosition.Y + 1 }); break;
		case 'C':  cursor({ Info.dwCursorPosition.X + 1, Info.dwCursorPosition.Y }); break;
		case 'D':  cursor({ Info.dwCursorPosition.X - 1, Info.dwCursorPosition.Y }); break;
		case 'H':  home_cursor(); break;
		case 'Y':
			// direct location?
			assert(false);
			break;
		case 'I': // reverse line feed humm
				  // reverse line feed humm
			assert(false);
			break;
		case 'K': // Erase to end of line	ESC K
			clear_from_pos(Info.dwCursorPosition, Info.srWindow.Right - Info.dwCursorPosition.X + 1);
			break;
		case 'J': // Erase to end of screen	ESC J
			clear_from_pos(Info.dwCursorPosition, Info.dwCursorPosition.Y*Info.dwSize.X + Info.dwCursorPosition.X + 1);
			break;
		case 'Z':
			// Identify?
			assert(false);
			break;// responce ESC / Z
		default:
			return false;
		}
		return true;
	}

	virtual void escape_dispatch(char c, const std::array<int, 16>& parms, const std::string& text)  override {
		switch (c) {
		case '<': std::cerr << "DEC52 mode" << std::endl; _dec52Mode = true; break;

		case 'D': std::cerr << "IND Index" << std::endl; break;
		case 'M': std::cerr << "RI Reverse Index" << std::endl; break;
		case 'E': std::cerr << "NEL next line" << std::endl; break;
		case '7':std::cerr << "Save cursor and atributes" << std::endl; break;
		case '8':std::cerr << "restore cursor and atributes" << std::endl; break;
		case 'N': std::cerr << "SS2" << std::endl; break;
		case 'O': std::cerr << "SS3" << std::endl; break;
		case 'P': std::cerr << "DCS" << std::endl; break;
		case '\\':  std::cerr << "ST" << std::endl; break; 	//Terminates strings in other controls(including APC, DCS, OSC, PM, and SOS)
		case ']':  std::cerr << "OSC" << std::endl; break; // operating system command
		case 'X': std::cerr << "SOS" << std::endl; break; // start of string
		case '^': std::cerr << "PM" << std::endl; break; // privacy message
		case '_': std::cerr << "APC" << std::endl; break; // privacy message
		case 'c': 
			std::cerr << "FULL_RESET" << std::endl; 
			home_cursor();
			_dec52Mode = false;
			SetConsoleTextAttribute(_conOut, _original.wAttributes);
			break; 
		default:
			if (_dec52Mode && !dec52(c,parms)) {
				std::cerr << "escape_dispatch: " << c << "(" << parms[0] << ", " << parms[1] << ")" << std::endl;

				assert(false);
			}
			
			break;
		}
		
	}
public:
	VT100() : _conOut(GetStdHandle(STD_OUTPUT_HANDLE)) { ::GetConsoleScreenBufferInfo(_conOut, &_original); home_cursor(); }
};









namespace vt100 {
	VT100 test;
	void print(char ch) { test.putch(ch); }
	void print(const char* message) {
		while (*message) test.putch(*message++);
	}
};