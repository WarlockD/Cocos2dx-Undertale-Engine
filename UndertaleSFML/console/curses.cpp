#include "curses.h"

namespace curses {
	

#define A_STANDOUT    (A_REVERSE | A_BOLD) /* X/Open */
#define A_DIM         A_NORMAL
 int          LINES;        /* terminal height */
	  int          COLS;         /* terminal width */
	  Window       *stdscr;      /* the default screen window */
	  Window       *curscr;      /* the current screen image */
									   //extern  SCREEN       *SP;          /* curses variables */
									   //extern  MOUSE_STATUS Mouse_status;
	  int          COLORS;
	  int          COLOR_PAIRS;
	  int          TABSIZE;
	  chtype       acs_map[];    /* alternate character set map */
	  char         ttytype[];    /* terminal name/description */

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
#ifdef XCURSES
		int   XcurscrSize;    /* size of Xcurscr shared memory block */
		bool  sb_on;
		int   sb_viewport_y;
		int   sb_viewport_x;
		int   sb_total_y;
		int   sb_total_x;
		int   sb_cur_y;
		int   sb_cur_x;
#endif
		short line_color;     /* color of line attributes - default -1 */
	};

};