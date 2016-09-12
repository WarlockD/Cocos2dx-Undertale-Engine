#pragma once
// functions included to access windows console
// coppied from pd curses, and combined into one header file just because.
#include <type_traits>

namespace curses {
	// keys
	typedef unsigned int chtype;
	constexpr chtype A_NORMAL = 0;
	constexpr chtype A_ALTCHARSET = sizeof(chtype) == 4 ? 0x00010000 : A_NORMAL;
	constexpr chtype A_RIGHTLINE = sizeof(chtype) == 4 ? 0x00020000 : A_NORMAL;
	constexpr chtype A_LEFTLINE = sizeof(chtype) == 4 ? 0x00040000 : A_NORMAL;
	constexpr chtype A_INVIS = sizeof(chtype) == 4 ? 0x00080000 : A_NORMAL;
	constexpr chtype A_UNDERLINE = sizeof(chtype) == 4 ? 0x00100000 : A_NORMAL;
	constexpr chtype A_REVERSE = sizeof(chtype) == 4 ? 0x00200000 : 0x0200;
	constexpr chtype A_BLINK = sizeof(chtype) == 4 ? 0x00400000 : 0x0400;
	constexpr chtype A_BOLD = sizeof(chtype) == 4 ? 0x00800000 : 0x0100;
	constexpr chtype A_ATTRIBUTES = sizeof(chtype) == 4 ? 0xffff0000 : 0xff00;
	constexpr chtype A_CHARTEXT = sizeof(chtype) == 4 ? 0x0000ffff : 0x00ff;
	constexpr chtype A_COLOR = sizeof(chtype) == 4 ? 0xff000000 : 0xf800;
	constexpr chtype A_ITALIC = sizeof(chtype) == 4 ? A_INVIS : A_NORMAL;
	constexpr chtype A_PROTECT = sizeof(chtype) == 4 ? (A_UNDERLINE | A_LEFTLINE | A_RIGHTLINE) : A_NORMAL;
	constexpr chtype PDC_ATTR_SHIFT = sizeof(chtype) == 4 ? 19 : 8;
	constexpr chtype PDC_COLOR_SHIFT = sizeof(chtype) == 4 ? 24 : 11;

	template<typename T, typename U>
	inline constexpr chtype ACS_PICK(T w, U n) { return sizeof(chtype) == 4 ? (static_cast<chtype>(w) | A_ALTCHARSET) : static_cast<chtype>(n); }
	/* VT100-compatible symbols -- box chars */
	constexpr chtype ACS_ULCORNER = ACS_PICK('l', '+');
	constexpr chtype ACS_LLCORNER = ACS_PICK('m', '+');
	constexpr chtype ACS_URCORNER = ACS_PICK('k', '+');
	constexpr chtype ACS_LRCORNER = ACS_PICK('j', '+');
	constexpr chtype ACS_RTEE = ACS_PICK('u', '+');
	constexpr chtype ACS_LTEE = ACS_PICK('t', '+');
	constexpr chtype ACS_BTEE = ACS_PICK('v', '+');
	constexpr chtype ACS_TTEE = ACS_PICK('w', '+');
	constexpr chtype ACS_HLINE = ACS_PICK('q', '-');
	constexpr chtype ACS_VLINE = ACS_PICK('x', '|');
	constexpr chtype ACS_PLUS = ACS_PICK('n', '+');

	/* VT100-compatible symbols -- other */
	constexpr chtype ACS_S1 = ACS_PICK('o', '-');
	constexpr chtype ACS_S9 = ACS_PICK('s', '_');
	constexpr chtype ACS_DIAMOND = ACS_PICK('`', '+');
	constexpr chtype ACS_CKBOARD = ACS_PICK('a', ':');
	constexpr chtype ACS_DEGREE = ACS_PICK('f', '\'');
	constexpr chtype ACS_PLMINUS = ACS_PICK('g', '#');
	constexpr chtype ACS_BULLET = ACS_PICK('~', 'o')
		;
	/*** Color macros ***/

	constexpr chtype COLOR_BLACK = 0;
	/* BGR */
	constexpr chtype COLOR_BLUE = 1;
	constexpr chtype COLOR_GREEN = 2;
	constexpr chtype COLOR_RED = 4;
	constexpr chtype COLOR_CYAN = (COLOR_BLUE | COLOR_GREEN);
	constexpr chtype COLOR_MAGENTA = (COLOR_RED | COLOR_BLUE);
	constexpr chtype COLOR_YELLOW = (COLOR_RED | COLOR_GREEN);
	constexpr chtype COLOR_WHITE = 7;

	constexpr chtype KEY_CODE_YES = 0x100; /* If get_wch() gives a key code */

	constexpr chtype KEY_BREAK = 0x101; /* Not on PC KBD */
	constexpr chtype KEY_DOWN = 0x102; /* Down arrow key */
	constexpr chtype KEY_UP = 0x103; /* Up arrow key */
	constexpr chtype KEY_LEFT = 0x104; /* Left arrow key */
	constexpr chtype KEY_RIGHT = 0x105; /* Right arrow key */
	constexpr chtype KEY_HOME = 0x106; /* home key */
	constexpr chtype KEY_BACKSPACE = 0x107; /* not on pc */
	constexpr chtype KEY_F0 = 0x108; /* function keys; 64 reserved */

	constexpr chtype KEY_DL = 0x148; /* delete line */
	constexpr chtype KEY_IL = 0x149; /* insert line */
	constexpr chtype KEY_DC = 0x14a; /* delete character */
	constexpr chtype KEY_IC = 0x14b; /* insert char or enter ins mode */
	constexpr chtype KEY_EIC = 0x14c; /* exit insert char mode */
	constexpr chtype KEY_CLEAR = 0x14d; /* clear screen */
	constexpr chtype KEY_EOS = 0x14e; /* clear to end of screen */
	constexpr chtype KEY_EOL = 0x14f; /* clear to end of line */
	constexpr chtype KEY_SF = 0x150; /* scroll 1 line forward */
	constexpr chtype KEY_SR = 0x151; /* scroll 1 line back (reverse) */
	constexpr chtype KEY_NPAGE = 0x152; /* next page */
	constexpr chtype KEY_PPAGE = 0x153; /* previous page */
	constexpr chtype KEY_STAB = 0x154; /* set tab */
	constexpr chtype KEY_CTAB = 0x155; /* clear tab */
	constexpr chtype KEY_CATAB = 0x156; /* clear all tabs */
	constexpr chtype KEY_ENTER = 0x157; /* enter or send (unreliable) */
	constexpr chtype KEY_SRESET = 0x158; /* soft/reset (partial/unreliable) */
	constexpr chtype KEY_RESET = 0x159; /* reset/hard reset (unreliable) */
	constexpr chtype KEY_PRINT = 0x15a; /* print/copy */
	constexpr chtype KEY_LL = 0x15b; /* home down/bottom (lower left) */
	constexpr chtype KEY_ABORT = 0x15c; /* abort/terminate key (any) */
	constexpr chtype KEY_SHELP = 0x15d; /* short help */
	constexpr chtype KEY_LHELP = 0x15e; /* long help */
	constexpr chtype KEY_BTAB = 0x15f; /* Back tab key */
	constexpr chtype KEY_BEG = 0x160; /* beg(inning) key */
	constexpr chtype KEY_CANCEL = 0x161; /* cancel key */
	constexpr chtype KEY_CLOSE = 0x162; /* close key */
	constexpr chtype KEY_COMMAND = 0x163; /* cmd (command) key */
	constexpr chtype KEY_COPY = 0x164; /* copy key */
	constexpr chtype KEY_CREATE = 0x165; /* create key */
	constexpr chtype KEY_END = 0x166; /* end key */
	constexpr chtype KEY_EXIT = 0x167; /* exit key */
	constexpr chtype KEY_FIND = 0x168; /* find key */
	constexpr chtype KEY_HELP = 0x169; /* help key */
	constexpr chtype KEY_MARK = 0x16a; /* mark key */
	constexpr chtype KEY_MESSAGE = 0x16b; /* message key */
	constexpr chtype KEY_MOVE = 0x16c; /* move key */
	constexpr chtype KEY_NEXT = 0x16d; /* next object key */
	constexpr chtype KEY_OPEN = 0x16e; /* open key */
	constexpr chtype KEY_OPTIONS = 0x16f; /* options key */
	constexpr chtype KEY_PREVIOUS = 0x170; /* previous object key */
	constexpr chtype KEY_REDO = 0x171; /* redo key */
	constexpr chtype KEY_REFERENCE = 0x172; /* ref(erence) key */
	constexpr chtype KEY_REFRESH = 0x173; /* refresh key */
	constexpr chtype KEY_REPLACE = 0x174; /* replace key */
	constexpr chtype KEY_RESTART = 0x175; /* restart key */
	constexpr chtype KEY_RESUME = 0x176; /* resume key */
	constexpr chtype KEY_SAVE = 0x177; /* save key */
	constexpr chtype KEY_SBEG = 0x178; /* shifted beginning key */
	constexpr chtype KEY_SCANCEL = 0x179; /* shifted cancel key */
	constexpr chtype KEY_SCOMMAND = 0x17a; /* shifted command key */
	constexpr chtype KEY_SCOPY = 0x17b; /* shifted copy key */
	constexpr chtype KEY_SCREATE = 0x17c; /* shifted create key */
	constexpr chtype KEY_SDC = 0x17d; /* shifted delete char key */
	constexpr chtype KEY_SDL = 0x17e; /* shifted delete line key */
	constexpr chtype KEY_SELECT = 0x17f; /* select key */
	constexpr chtype KEY_SEND = 0x180; /* shifted end key */
	constexpr chtype KEY_SEOL = 0x181; /* shifted clear line key */
	constexpr chtype KEY_SEXIT = 0x182; /* shifted exit key */
	constexpr chtype KEY_SFIND = 0x183; /* shifted find key */
	constexpr chtype KEY_SHOME = 0x184; /* shifted home key */
	constexpr chtype KEY_SIC = 0x185; /* shifted input key */

	constexpr chtype KEY_SLEFT = 0x187; /* shifted left arrow key */
	constexpr chtype KEY_SMESSAGE = 0x188; /* shifted message key */
	constexpr chtype KEY_SMOVE = 0x189; /* shifted move key */
	constexpr chtype KEY_SNEXT = 0x18a; /* shifted next key */
	constexpr chtype KEY_SOPTIONS = 0x18b; /* shifted options key */
	constexpr chtype KEY_SPREVIOUS = 0x18c; /* shifted prev key */
	constexpr chtype KEY_SPRINT = 0x18d; /* shifted print key */
	constexpr chtype KEY_SREDO = 0x18e; /* shifted redo key */
	constexpr chtype KEY_SREPLACE = 0x18f; /* shifted replace key */
	constexpr chtype KEY_SRIGHT = 0x190; /* shifted right arrow */
	constexpr chtype KEY_SRSUME = 0x191; /* shifted resume key */
	constexpr chtype KEY_SSAVE = 0x192; /* shifted save key */
	constexpr chtype KEY_SSUSPEND = 0x193; /* shifted suspend key */
	constexpr chtype KEY_SUNDO = 0x194; /* shifted undo key */
	constexpr chtype KEY_SUSPEND = 0x195; /* suspend key */
	constexpr chtype KEY_UNDO = 0x196; /* undo key */

	/* PDCurses-specific key definitions -- PC only */

	constexpr chtype ALT_0 = 0x197;
	constexpr chtype ALT_1 = 0x198;
	constexpr chtype ALT_2 = 0x199;
	constexpr chtype ALT_3 = 0x19a;
	constexpr chtype ALT_4 = 0x19b;
	constexpr chtype ALT_5 = 0x19c;
	constexpr chtype ALT_6 = 0x19d;
	constexpr chtype ALT_7 = 0x19e;
	constexpr chtype ALT_8 = 0x19f;
	constexpr chtype ALT_9 = 0x1a0;
	constexpr chtype ALT_A = 0x1a1;
	constexpr chtype ALT_B = 0x1a2;
	constexpr chtype ALT_C = 0x1a3;
	constexpr chtype ALT_D = 0x1a4;
	constexpr chtype ALT_E = 0x1a5;
	constexpr chtype ALT_F = 0x1a6;
	constexpr chtype ALT_G = 0x1a7;
	constexpr chtype ALT_H = 0x1a8;
	constexpr chtype ALT_I = 0x1a9;
	constexpr chtype ALT_J = 0x1aa;
	constexpr chtype ALT_K = 0x1ab;
	constexpr chtype ALT_L = 0x1ac;
	constexpr chtype ALT_M = 0x1ad;
	constexpr chtype ALT_N = 0x1ae;
	constexpr chtype ALT_O = 0x1af;
	constexpr chtype ALT_P = 0x1b0;
	constexpr chtype ALT_Q = 0x1b1;
	constexpr chtype ALT_R = 0x1b2;
	constexpr chtype ALT_S = 0x1b3;
	constexpr chtype ALT_T = 0x1b4;
	constexpr chtype ALT_U = 0x1b5;
	constexpr chtype ALT_V = 0x1b6;
	constexpr chtype ALT_W = 0x1b7;
	constexpr chtype ALT_X = 0x1b8;
	constexpr chtype ALT_Y = 0x1b9;
	constexpr chtype ALT_Z = 0x1ba;

	constexpr chtype CTL_LEFT = 0x1bb; /* Control-Left-Arrow */
	constexpr chtype CTL_RIGHT = 0x1bc;
	constexpr chtype CTL_PGUP = 0x1bd;
	constexpr chtype CTL_PGDN = 0x1be;
	constexpr chtype CTL_HOME = 0x1bf;
	constexpr chtype CTL_END = 0x1c0;

	constexpr chtype KEY_A1 = 0x1c1; /* upper left on Virtual keypad */
	constexpr chtype KEY_A2 = 0x1c2; /* upper middle on Virt. keypad */
	constexpr chtype KEY_A3 = 0x1c3; /* upper right on Vir. keypad */
	constexpr chtype KEY_B1 = 0x1c4; /* middle left on Virt. keypad */
	constexpr chtype KEY_B2 = 0x1c5; /* center on Virt. keypad */
	constexpr chtype KEY_B3 = 0x1c6; /* middle right on Vir. keypad */
	constexpr chtype KEY_C1 = 0x1c7; /* lower left on Virt. keypad */
	constexpr chtype KEY_C2 = 0x1c8; /* lower middle on Virt. keypad */
	constexpr chtype KEY_C3 = 0x1c9; /* lower right on Vir. keypad */

	constexpr chtype PADSLASH = 0x1ca; /* slash on keypad */
	constexpr chtype PADENTER = 0x1cb; /* enter on keypad */
	constexpr chtype CTL_PADENTER = 0x1cc; /* ctl-enter on keypad */
	constexpr chtype ALT_PADENTER = 0x1cd; /* alt-enter on keypad */
	constexpr chtype PADSTOP = 0x1ce; /* stop on keypad */
	constexpr chtype PADSTAR = 0x1cf; /* star on keypad */
	constexpr chtype PADMINUS = 0x1d0; /* minus on keypad */
	constexpr chtype PADPLUS = 0x1d1; /* plus on keypad */
	constexpr chtype CTL_PADSTOP = 0x1d2; /* ctl-stop on keypad */
	constexpr chtype CTL_PADCENTER = 0x1d3; /* ctl-enter on keypad */
	constexpr chtype CTL_PADPLUS = 0x1d4; /* ctl-plus on keypad */
	constexpr chtype CTL_PADMINUS = 0x1d5; /* ctl-minus on keypad */
	constexpr chtype CTL_PADSLASH = 0x1d6; /* ctl-slash on keypad */
	constexpr chtype CTL_PADSTAR = 0x1d7; /* ctl-star on keypad */
	constexpr chtype ALT_PADPLUS = 0x1d8; /* alt-plus on keypad */
	constexpr chtype ALT_PADMINUS = 0x1d9; /* alt-minus on keypad */
	constexpr chtype ALT_PADSLASH = 0x1da; /* alt-slash on keypad */
	constexpr chtype ALT_PADSTAR = 0x1db; /* alt-star on keypad */
	constexpr chtype ALT_PADSTOP = 0x1dc; /* alt-stop on keypad */
	constexpr chtype CTL_INS = 0x1dd; /* ctl-insert */
	constexpr chtype ALT_DEL = 0x1de; /* alt-delete */
	constexpr chtype ALT_INS = 0x1df; /* alt-insert */
	constexpr chtype CTL_UP = 0x1e0; /* ctl-up arrow */
	constexpr chtype CTL_DOWN = 0x1e1; /* ctl-down arrow */
	constexpr chtype CTL_TAB = 0x1e2; /* ctl-tab */
	constexpr chtype ALT_TAB = 0x1e3;
	constexpr chtype ALT_MINUS = 0x1e4;
	constexpr chtype ALT_EQUAL = 0x1e5;
	constexpr chtype ALT_HOME = 0x1e6;
	constexpr chtype ALT_PGUP = 0x1e7;
	constexpr chtype ALT_PGDN = 0x1e8;
	constexpr chtype ALT_END = 0x1e9;
	constexpr chtype ALT_UP = 0x1ea; /* alt-up arrow */
	constexpr chtype ALT_DOWN = 0x1eb; /* alt-down arrow */
	constexpr chtype ALT_RIGHT = 0x1ec; /* alt-right arrow */
	constexpr chtype ALT_LEFT = 0x1ed; /* alt-left arrow */
	constexpr chtype ALT_ENTER = 0x1ee; /* alt-enter */
	constexpr chtype ALT_ESC = 0x1ef; /* alt-escape */
	constexpr chtype ALT_BQUOTE = 0x1f0; /* alt-back quote */
	constexpr chtype ALT_LBRACKET = 0x1f1; /* alt-left bracket */
	constexpr chtype ALT_RBRACKET = 0x1f2; /* alt-right bracket */
	constexpr chtype ALT_SEMICOLON = 0x1f3; /* alt-semi-colon */
	constexpr chtype ALT_FQUOTE = 0x1f4; /* alt-forward quote */
	constexpr chtype ALT_COMMA = 0x1f5; /* alt-comma */
	constexpr chtype ALT_STOP = 0x1f6; /* alt-stop */
	constexpr chtype ALT_FSLASH = 0x1f7; /* alt-forward slash */
	constexpr chtype ALT_BKSP = 0x1f8; /* alt-backspace */
	constexpr chtype CTL_BKSP = 0x1f9; /* ctl-backspace */
	constexpr chtype PAD0 = 0x1fa; /* keypad 0 */

	constexpr chtype CTL_PAD0 = 0x1fb; /* ctl-keypad 0 */
	constexpr chtype CTL_PAD1 = 0x1fc;
	constexpr chtype CTL_PAD2 = 0x1fd;
	constexpr chtype CTL_PAD3 = 0x1fe;
	constexpr chtype CTL_PAD4 = 0x1ff;
	constexpr chtype CTL_PAD5 = 0x200;
	constexpr chtype CTL_PAD6 = 0x201;
	constexpr chtype CTL_PAD7 = 0x202;
	constexpr chtype CTL_PAD8 = 0x203;
	constexpr chtype CTL_PAD9 = 0x204;

	constexpr chtype ALT_PAD0 = 0x205; /* alt-keypad 0 */
	constexpr chtype ALT_PAD1 = 0x206;
	constexpr chtype ALT_PAD2 = 0x207;
	constexpr chtype ALT_PAD3 = 0x208;
	constexpr chtype ALT_PAD4 = 0x209;
	constexpr chtype ALT_PAD5 = 0x20a;
	constexpr chtype ALT_PAD6 = 0x20b;
	constexpr chtype ALT_PAD7 = 0x20c;
	constexpr chtype ALT_PAD8 = 0x20d;
	constexpr chtype ALT_PAD9 = 0x20e;

	constexpr chtype CTL_DEL = 0x20f; /* clt-delete */
	constexpr chtype ALT_BSLASH = 0x210; /* alt-back slash */
	constexpr chtype CTL_ENTER = 0x211; /* ctl-enter */

	constexpr chtype SHF_PADENTER = 0x212; /* shift-enter on keypad */
	constexpr chtype SHF_PADSLASH = 0x213; /* shift-slash on keypad */
	constexpr chtype SHF_PADSTAR = 0x214; /* shift-star  on keypad */
	constexpr chtype SHF_PADPLUS = 0x215; /* shift-plus  on keypad */
	constexpr chtype SHF_PADMINUS = 0x216; /* shift-minus on keypad */
	constexpr chtype SHF_UP = 0x217; /* shift-up on keypad */
	constexpr chtype SHF_DOWN = 0x218; /* shift-down on keypad */
	constexpr chtype SHF_IC = 0x219; /* shift-insert on keypad */
	constexpr chtype SHF_DC = 0x21a; /* shift-delete on keypad */

	constexpr chtype KEY_MOUSE = 0x21b; /* "mouse" key */
	constexpr chtype KEY_SHIFT_L = 0x21c; /* Left-shift */
	constexpr chtype KEY_SHIFT_R = 0x21d; /* Right-shift */
	constexpr chtype KEY_CONTROL_L = 0x21e; /* Left-control */
	constexpr chtype KEY_CONTROL_R = 0x21f; /* Right-control */
	constexpr chtype KEY_ALT_L = 0x220; /* Left-alt */
	constexpr chtype KEY_ALT_R = 0x221; /* Right-alt */
	constexpr chtype KEY_RESIZE = 0x222; /* Window resize */
	constexpr chtype KEY_SUP = 0x223; /* Shifted up arrow */
	constexpr chtype KEY_SDOWN = 0x224; /* Shifted down arrow */

	constexpr chtype KEY_MIN = KEY_BREAK; /* Minimum curses key value */
	constexpr chtype KEY_MAX = KEY_SDOWN; /* Maximum curses key */
	// why use macros when constexpr works just as well?
	template<typename T>
	constexpr chtype KEY_F(T v) { return static_cast<chtyp>(v) + KEY_F0; }
	extern int COLS;
	extern int LINES;
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
		void wnoutrefresh();
		bool wrefresh();
		bool wredrawln(int start, int num);
		bool redrawwin();
	};
	bool refresh();
	bool doupdate();

	namespace priv {
		struct Screen
		{
			bool  alive;          /* if initscr() called, and not endwin() */
			bool  autocr;         /* if cr -> lf */
			bool  cbreak;         /* if terminal unbuffered */
			bool  echo;           /* if terminal echo */
			bool  raw_inp;        /* raw input mode (v. cooked input) */
			bool  raw_out;        /* raw output mode (7 v. 8 bits) */
			bool  audible;        /* FALSE if the bell is visual */
			bool  mono;           /* TRUE if current screen is mono */
			bool  resized;        /* TRUE if TERM has been resized */
			bool  orig_attr;      /* TRUE if we have the original colors */
			short orig_fore;      /* original screen foreground color */
			short orig_back;      /* original screen foreground color */
			int   cursrow;        /* position of physical cursor */
			int   curscol;        /* position of physical cursor */
			int   visibility;     /* visibility of cursor */
			int   orig_cursor;    /* original cursor size */
			int   lines;          /* new value for LINES */
			int   cols;           /* new value for COLS */
			unsigned long _trap_mbe;       /* trap these mouse button events */
			unsigned long _map_mbe_to_key; /* map mouse buttons to slk */
			int   mouse_wait;              /* time to wait (in ms) for a
										   button release after a press, in
										   order to count it as a click */
			int   slklines;                /* lines in use by slk_init() */
		//	WINDOW *slk_winptr;            /* window for slk */
			int   linesrippedoff;          /* lines ripped off via ripoffline() */
			int   linesrippedoffontop;     /* lines ripped off on
										   top via ripoffline() */
			int   delaytenths;             /* 1/10ths second to wait block
										   getch() for */
			bool  _preserve;               /* TRUE if screen background
										   to be preserved */
			int   _restore;                /* specifies if screen background
										   to be restored, and how */
			bool  save_key_modifiers;      /* TRUE if each key modifiers saved
										   with each key press */
			bool  return_key_modifiers;    /* TRUE if modifier keys are
										   returned as "real" keys */
			bool  key_code;                /* TRUE if last key is a special key;
										   used internally by get_wch() */
			short line_color;     /* color of line attributes - default -1 */
		};
		bool check_key();
		bool scr_open();
		int get_rows();
		int get_columns();
		bool resize_screen(int nlines, int ncols);
		void reset_prog_mode();
		int get_buffer_rows();
		void mouse_set();
		void reset_shell_mode();
		void gotoyx(int row, int col);
		void transform_line(int lineno, int x, int len, const chtype *srcp);
		void init_pair(short pair, short fg, short bg);
		void pair_content(short pair, short *fg, short *bg);
		void color_content(short color, short *red, short *green, short *blue);
		void init_color(short color, short red, short green, short blue);
	};
};