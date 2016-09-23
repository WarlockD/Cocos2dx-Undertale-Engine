#include "console.h"
#include <Windows.h>
#include <set>
#include <algorithm>
#include <utility>
#include <varargs.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace {
	static CONSOLE_SCREEN_BUFFER_INFO s_info;
static HANDLE s_hStdOut = INVALID_HANDLE_VALUE;
static void UpdateInfo() {
	if (s_hStdOut == INVALID_HANDLE_VALUE) s_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	assert(s_hStdOut != INVALID_HANDLE_VALUE);
	GetConsoleScreenBufferInfo(s_hStdOut, &s_info);
}
};


namespace con {
	enum class LineFlags : size_t {
		Zero = 0x00,
		IsDirty = 0x01,		/* Line is dirty. */
		IsPastEol = 0x02,		/* Cursor is past end of line */
		ForcePaint = 0x04,		/* Force a repaint of the line */
	};
	ENUM_OPERATIONS(LineFlags);
	enum class WindowFlags : size_t {
		Zero = 0x00,
		EndLine = 0x001,		/* End of screen. */
		Flush = 0x002,		/* Fflush(stdout) after refresh. */
		FullLine = 0x004,		/* Line width = terminal width. */
		FullWin = 0x008,		/* Window is a screen. */
		IdLine = 0x010,		/* Insert/delete sequences. */
		ScrollWin = 0x020,		/* Last char will scroll window. */
		ScrollOk = 0x040,		/* Scrolling ok. */
		ClearOk = 0x080,		/* Clear on next refresh. */
		WStandout = 0x100,		/* Standout window */
		LeaveOk = 0x200		/* If curser left */
	};
	ENUM_OPERATIONS(WindowFlags);

	int LINES() {
		UpdateInfo();
		return s_info.dwSize.Y;
	}
	int COLS() {
		UpdateInfo();
		return s_info.dwSize.X;
	}
	// hash from here
	// http://minnie.tuhs.org/cgi-bin/utree.pl?file=4.4BSD/usr/src/lib/libcurses/hash.c
	size_t s_hash(const char*s, size_t len) {
		register size_t	h = 0, g = 0;
		for (size_t i = 0; i < len; i++) {
			h = (h << 4) + s[i];
			if (g = h & 0xf0000000) {
				h = h ^ (g >> 24);
				h = h ^ g;
			}
		}
		return h;
	}

	inline bool operator==(const CHAR_INFO& l, const CHAR_INFO& r) { return l.Char.AsciiChar == r.Char.AsciiChar && l.Attributes == r.Attributes; }
	inline bool operator!=(const CHAR_INFO& l, const CHAR_INFO& r) { return !(l == r); }
	struct Line {
		typedef std::vector<CHAR_INFO>::iterator char_iterator;
		typedef std::vector<CHAR_INFO>::const_iterator char_const_iterator;
		std::vector<CHAR_INFO> line;
		char_iterator first;
		char_iterator last;
		char_iterator begin() { return line.begin(); }
		char_iterator end() { return line.end(); }
		void touch() { first = line.begin(); last = line.end() - 1; }
		void untouch() { first = last = line.end(); }
		Line(size_t len, const CHAR_INFO& ch) : line(len, ch) { touch(); }
		Line(const Line& copy) :line(copy.line) { touch(); }
		Line& operator=(const Line& copy) { line = copy.line; touch(); return *this; }
		Line(Line&& move) { std::swap(move.line, line);  move.touch(); touch(); }
		Line& operator=(Line&& move) { std::swap(move.line, line); touch(); move.touch(); return *this; }
		void fill(const CHAR_INFO& ch) {
			std::fill(line.begin(), line.end(), ch);
			touch();
		}
		void fill(const CHAR_INFO& ch, short pos) {
			char_iterator start = line.begin() + pos;
			std::fill(start, line.end(), ch);
			if (!isDirty() || first > start) first = start;
			last = line.end() - 1;
		}
		void fill(const CHAR_INFO& ch, short pos, short len) {
			char_iterator start = line.begin() + pos;
			char_iterator end = unsigned(pos + len) > line.size() ? line.end() : (line.begin() + pos + len);
			std::fill(start, end, ch);
			if (!isDirty()) { first = start; last = end-1; }
			else {
				end--;
				if (first > start) first = start;
				if (last < end) last = end;
			}
			
		}
		bool isDirty() const { return first == line.end(); }
		const CHAR_INFO& at(size_t pos) const { return line[pos]; }
		inline void change(char_iterator it, const CHAR_INFO& ch) {
			if (it >= line.end()) it = line.end() - 1;
			if (ch != *it) {
				*it = ch;
				if (first == line.end())
					first = last = it;
				else
					if (it < first) first = it;
					else if (it > last) last = it;
			}
		}
		inline void change(size_t pos, const CHAR_INFO& ch) {
			change(line.begin() + pos, ch);
		}
		// copys line to here with offset from line
		inline void change(char_iterator from, char_iterator to, char_const_iterator data) {
			for (; from != to; from++) change(from, *data++);
		}
	};
	void PDC_transform_line(int lineno, int x, int len, const CHAR_INFO *ci)
	{
		COORD bufSize, bufPos;
		SMALL_RECT sr;

		bufPos.X = bufPos.Y = 0;

		bufSize.X = len;
		bufSize.Y = 1;

		sr.Top = lineno;
		sr.Bottom = lineno;
		sr.Left = x;
		sr.Right = x + len - 1;

		WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), ci, bufSize, bufPos, &sr);
	}

	VWindow::VWindow(size_t width, size_t height) : VWindow(0,0,width,height) { }
	VWindow::VWindow(size_t x, size_t y, size_t width, size_t height) : _begx(x), _begy(y), _curx(0), _cury(0), _maxx(width), _maxy(height) , _need_refresh(true){}

	void VWindow::print(const char* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		char buffer[256];
		auto count = vsnprintf(buffer, 256, fmt, va);
		va_end(va);
		this->puts(buffer);

		//_stream.write(buffer, count);
	}
	void VWindow::background(console::Color color) {

	}
	void VWindow::forground(console::Color color) {

	}
	
	void VWindow::scroll(int n) {
		// scroll does nothing
	}

	
	void VWindow::linefeed() {
		if (++_cury >= _maxy) { scroll(1); _cury = _maxy - 1; }
	}

	void VWindow::putc(char ch) { 
		switch (ch) {
		case '\n': if (_cury++ >= _maxy) { _cury = _maxy - 1; } _need_refresh = true; break;
		case '\r': if (_curx != 0) { _need_refresh = true; _curx = 0; } break;
		case '\b': if (_curx != 0) { _curx--; _buffer << '\b'; } break;
		default:
			_refresh();
			_buffer << ch;
			if(_curx++ >= _maxx) { _curx = _maxx - 1; }
			break;
		}
	}
	void VWindow::_refresh() {
		if(_need_refresh) {
			_buffer << "\x1b[" << (_cury + _begy) << ';' << (_curx + _begx) << 'H';
			_need_refresh = false;
		}
	}
	void  VWindow::move(size_t x, size_t y) {
		_curx = std::min(x, _maxx);
		_cury = std::min(y, _maxy);
		_need_refresh = true;
	}
	void  VWindow::erase_to_eol() {
		int count = _maxx - _curx - 1;
		if (count > 0) {
			_refresh();
			_buffer << "\x1b[" << _maxx - _curx - 1 << 'X';
		}
	}
	void  VWindow::erase_to_bot() {
		erase_to_eol();
		_buffer << L"\x1b[s"; // save cursor
		_buffer << L"\x1b[" << (_curx + _begx) << 'G'; // 
		for (size_t y = _cury + 1; y < _maxy; y++) {
			_buffer << "\x1b[" << (y + _begy) << 'd';
			_buffer << "\x1b[" << _maxx - _curx - 1 << 'X';
		}
		_buffer << "\x1b[u"; // restore cursor
	}
	void VWindow::refresh(bool clearall) {
		auto& str = _buffer.str();
		if (str.length() > 0) {
			std::cout << "x1b[?25h"; //hide the cursor then refresh
			std::cout << str;
			std::cout << "x1b[?25l";
			_buffer.clear();
		}	
	}
	class BaseWindow : public con::Window {
	protected:
		COORD _cur;
		COORD _max;
		COORD _beg;
		short _bmarg;
		short _tmarg;
		short _lmarg;
		short _rmarg;
		WORD _cattrib;
		bool  _clear;         /* causes clear at next refresh */
		bool  _leaveit;       /* leaves cursor where it is */
		bool  _scroll;        /* allows window scrolling */
		bool  _nodelay;       /* input character wait flag */
		bool  _immed;         /* immediate update flag */
		bool  _sync;          /* synchronise window ancestors */
		BaseWindow* _parent;
		std::list<std::weak_ptr<Window>> _windows;
		std::vector<Line> _lines;
		typedef std::vector<Line>::iterator line_iterator;
		std::set<short> _tabs;
		short next_tab() {
			if (_tabs.empty()) return 8 + (_cur.X % 8); // default to 8 spaces per tab
			for (auto s : _tabs) if (s >= _cur.X && s < _max.X) return s + (_cur.X % s);
			return _max.X - 1;
		}
		CHAR_INFO blank() const { return{ wchar_t(' '), _cattrib }; }
		void sync() {
			if (_immed) refresh();
			if (_sync) syncup();
		}
	
	public:
		bool is_linetouched(int line) const {
			if (line > _max.Y || line < 0) return false;
			return _lines.at(line).isDirty();
		}
		bool is_wintouched() {
			for (auto& l : _lines) if (l.isDirty()) return true;
			return false;
		}
		void touchwin() {
			std::for_each(_lines.begin(), _lines.end(), [this](Line& l) { l.touch(); });
		}
		void untouchwin() {
			std::for_each(_lines.begin(), _lines.end(), [](Line& l) { l.untouch(); });
		}
		void touchln(int y, int n, bool changed) {
			if (y < _max.Y && y >= 0) {
				if (changed)
					std::for_each(_lines.begin() + y, _lines.end() + n, [this](Line& l) { l.touch();; });
				else
					std::for_each(_lines.begin() + y, _lines.end() + n, [](Line& l) { l.untouch(); });
			}
		}
		
		void touchline(int y, int n) { touchln(y, n, true); }
		void touchline(int y, int n, bool changed) { touchln(y, n, changed); }
		BaseWindow(size_t x, size_t y, size_t width, size_t height) :
			_cur{ 0,0 },
			_max{ short(width == 0 ? s_info.dwSize.X - x : width),  short(height == 0 ? s_info.dwSize.Y - y : height) },
			_beg{ short(x),short(y) },
			_tmarg(0), _bmarg(_max.Y-1), _lmarg(0), _rmarg(_max.X-1),
			_cattrib(0x07), _clear(false), _leaveit(false), _scroll(true), _nodelay(false), _immed(false), _sync(false) , _parent(nullptr) {
			_lines.resize(_max.Y,Line(_max.X,blank()));
		}
		BaseWindow(BaseWindow* parent, size_t x, size_t y, size_t width, size_t height) :
			_cur{ 0,0 },
			_max{ short(width == 0 ? s_info.dwSize.X  - _parent->_beg.X - x : width),  short(height == 0 ? s_info.dwSize.Y- _parent->_beg.Y - y : height) },
			_beg{ short(x),short(y) },
			_tmarg(0), _bmarg(_max.Y - 1), _lmarg(0), _rmarg(_max.X - 1),
			_cattrib(parent->_cattrib), _clear(parent->_clear), _leaveit(parent->_leaveit), _scroll(parent->_scroll), _nodelay(parent->_nodelay), _immed(parent->_immed), _sync(parent->_sync), _parent(parent) {
			_lines.resize(_max.Y, Line(_max.X, blank()));
		}
		void move(size_t x, size_t y) override {
			if (x < unsigned(_max.X) && y < unsigned(_max.Y)) {
				_cur.X = short(x);
				_cur.Y = short(y);
			}
		}
		void addch(int c) override {
			// alt charset here
			if (c < ' ' || c == 0x7f) {
				switch (c) {
				case '\t':
				{
					auto spaces = next_tab();
					while (spaces--) {
						addch(' ');
						if (!_cur.X) break; // end of line break
					}
				}
					break;
				case '\n':
					clrtoeol();
					if ((_cur.Y + 1) > _bmarg) scroll(1);
					else _cur.Y++;
					break;
				case '\b':
					/* don't back over left margin */
					if (--_cur.X < 0) _cur.X = 0;// reall nutty here... humm
					break;
				case '\r':
					_cur.X = 0;
					break;
				case 0x7f:
					addch('^');
					addch('?');
					break;
				default:
					/* handle control chars */
					addch('^');
					addch('@' + c);
					break;
				}
			} else {
				CHAR_INFO ch = { wchar_t(c), _cattrib };
				_lines.at(_cur.Y).change(_cur.X, ch);
				if (++_cur.X >= _max.X) {
					_cur.X = 0;
					if ((_cur.Y + 1) > _bmarg) scroll(1);
					else _cur.Y++;
				}
			}
			sync();
		}
		void inch(int c) override  { 
			if (c < ' ' || c == 0x7f) {
				switch (c) {
				case '\t':
				{
					auto spaces = next_tab();
					while (spaces--) inch(' ');
				}
					
					break;
				case '\n':
					clrtoeol();
					break;
				case 0x7f:
					inch('?');
					inch('^');
					break;
				default:
					/* handle control chars */
					inch('@' + c);
					inch('^');
					break;
				}
			} else {
				CHAR_INFO ch = { wchar_t(c), _cattrib };
				auto &line = _lines.at(_cur.Y);
				auto it = line.begin() + _cur.X;
				std::move(it, line.end() - 1, it + 1);
				*it = ch;
				if (line.isDirty() || line.first > it) line.first = it;
				line.last = line.end() - 1;
			}
			sync();
		}
		size_t lines() const override { return _max.Y; }
		size_t cols() const override { return _max.X; }
		void clrtoeol() override {
			auto& line = _lines.at(_cur.Y);
			line.fill(blank(), _cur.X);
			sync();
		}
		void clrtobot() override {
			COORD save = _cur;
			for (size_t i = _cur.Y + 1; i < unsigned(_max.Y); i++)
				_lines[i].fill(blank());
			clrtoeol();
		}
		void syncok(bool bf) override { _sync = bf; }
		void syncup() override {
			for (BaseWindow* tmp = this; tmp; tmp = tmp->_parent) 
				tmp->touchwin();
		}
		void cursyncup() override {
			for (auto tmp = this; tmp && tmp->_parent; tmp = tmp->_parent) {
				COORD par = tmp->_parent->_cur;
				tmp->_parent->move(par.Y + tmp->_cur.Y, par.X + tmp->_cur.X);
			}
				
		}
		void syncdown() override {
			for (auto tmp = this; tmp; tmp = tmp->_parent)
				if (tmp->is_wintouched())
				{
					touchwin();
					break;
				}
		}
		void scroll(int n) override {
			size_t count = std::abs(n);
			std::vector<Line> backup;
			backup.reserve(count);
			if (n > 0)
			{
				_lines.erase(_lines.begin() + _tmarg, _lines.begin() + _tmarg+ count);
				while (n--) _lines.emplace_back(_max.X, blank());
				touchline(_tmarg, _bmarg - _tmarg + 1);
			}
			else if(n < 0)
			{
				while (n++) _lines.emplace(_lines.begin(), _max.X, blank());
				_lines.erase(_lines.begin() + _bmarg, _lines.begin() + _bmarg + count);
				touchline(_tmarg, _bmarg - _tmarg + 1);
			}
			
		}
		void insertln() override {
			_lines.emplace(_lines.begin() + _cur.Y, _max.X, blank());
			_lines.erase(_lines.end());
		}
		void deleteln() override {
			_lines.erase(_lines.begin() + _cur.Y);
			_lines.emplace_back(_max.X, blank());
		}
		void update(bool clearall = false) override {
			refresh();
			std::vector<CHAR_INFO> lbuffer;
			lbuffer.reserve(_max.X);
			for (size_t y = 0; y < unsigned(_max.Y); y++)
			{
				auto& line = _lines.at(y);
				if (line.isDirty()) {
					size_t first = std::distance(line.begin(), line.first);
					size_t last = std::distance(line.begin(), line.last);
					int len = last - first + 1;
					PDC_transform_line(y, first, len, line.line.data() + first);
					line.untouch();
				}
			}
		}
		void refresh() override {
			for (auto it = _windows.begin(); it != _windows.end(); it++) {
				auto& ptr = *it;
				if (ptr.expired()) it = _windows.erase(it);
				else ptr.lock()->refresh();
			}
			if (_parent == nullptr) return;
			for (size_t i = 0, j = _beg.Y; i < unsigned(_max.Y); i++, j++)
			{
				auto& line = _lines.at(i);
				if (!line.isDirty())
				{
					auto& pline = _parent->_lines.at(j);
					Line::char_iterator dest = pline.begin() + _beg.X;
					Line::char_iterator src = line.begin();
					size_t first = std::distance(line.begin(), line.first);
					size_t last = std::distance(line.begin(), line.last);

					while (first <= last && src[first] == dest[first]) first++;
					while (last <= first && src[last] == dest[last]) last--;
					if (first <= last) /* if any have really changed... */
					{
						first += _beg.X; last += _beg.X;
						pline.change(dest + first, dest + last, line.begin() + first);
					}
					line.untouch();

					while (dest != pline.begin() + _max.X) {
						if (line.first <= line.last &&   *src == *dest) {
							*dest = *src;

						}
					}
				}
			}
			if (_clear) _clear = false;
			if (!_leaveit) _parent->_cur = { _cur.X + _beg.X,_cur.Y + _beg.Y }; 
		}
		std::shared_ptr<Window> subwin(size_t x, size_t y, size_t width, size_t height) override {
			std::shared_ptr<Window> ret;
			if (x < unsigned(_beg.X) || y < unsigned(_beg.Y) || (x + width) >unsigned(_max.X + _beg.X) || (y + height) >unsigned(_max.Y + _beg.Y)) return ret; // empty pointer
			if (height == 0) height = _max.Y + _beg.Y - y;
			if (width == 0) width = _max.X + _beg.X - x;
			ret.reset(new BaseWindow(this, x, y, width, height));
			for (auto& window : _windows) {
				if (window.expired()) {
					window = ret;
					return ret;
				}
			}
			_windows.emplace_back(ret);
			return ret;
		}
		virtual ~BaseWindow() {}
	};
	
		//void fffrefresh()  {
			//sync();
		//	COORD pos = { 0,0 };
		//	SMALL_RECT rect = { 0, 0, _max.X - 1, _max.Y - 1 };
		//	WriteConsoleOutputA(GetStdHandle(STD_OUTPUT_HANDLE), _wspace.data(), _max, pos, &rect);
		//}

	std::shared_ptr<Window> newwin(size_t x, size_t y, size_t width, size_t height) {
		return std::shared_ptr<Window>(new BaseWindow(x, y, width, height));
	}

};