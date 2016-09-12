#include "priv_console.h"

#include <Windows.h>
#include <wincon.h>
#include <vector>

unsigned long pdc_key_modifiers = 0L;

/* These variables are used to store information about the next
Input Event. */
namespace curses {
	int COLS = 0;
	int LINES = 0;
	static HANDLE pdc_con_in = INVALID_HANDLE_VALUE;
	static HANDLE pdc_con_out = INVALID_HANDLE_VALUE;
	static DWORD pdc_quick_edit = 0;
	static Window* curscr = nullptr;
	static Window* pdc_lastscr = nullptr;
	/* Internal macros for attributes */
	constexpr chtype PDC_COLOR_PAIRS = sizeof(chtype) == 4 ? 256 : 32;
	constexpr chtype PDC_OFFSET = sizeof(chtype) == 4 ? 32 : 8;
	constexpr chtype PDC_CLICK_PERIOD = 150;  /* time to wait for a click, if not set by mouseinterval() */

	template<typename T>
	constexpr T DIVROUND(const T num, const T divisor) { return ((num)+((divisor) >> 1)) / (divisor); }

	/* COLOR_PAIR to attribute encoding table. */

	std::vector<unsigned char> pdc_atrtab;

	static short curstoreal[16], realtocurs[16] =
	{
		COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED,
		COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE, COLOR_BLACK + 8,
		COLOR_BLUE + 8, COLOR_GREEN + 8, COLOR_CYAN + 8, COLOR_RED + 8,
		COLOR_MAGENTA + 8, COLOR_YELLOW + 8, COLOR_WHITE + 8
	};
	enum { PDC_RESTORE_NONE, PDC_RESTORE_BUFFER, PDC_RESTORE_WINDOW };
	struct   RIPPEDOFFLINE         /* structure for ripped off lines */
	{
		int line;
		int(*init)(Window *, int);
	} ;

	/* Window properties */

	constexpr chtype _SUBWIN = 0x01;  /* window is a subwindow */
	constexpr chtype _PAD = 0x10;  /* X/Open Pad. */
	constexpr chtype _SUBPAD = 0x20;  /* X/Open subpad. */

	/* Miscellaneous */

	constexpr chtype _NO_CHANGE = -1;    /* flags line edge unchanged */

	constexpr chtype _ECHAR = 0x08;  /* Erase char       (^H) */
	constexpr chtype _DWCHAR = 0x17;  /* Delete Word char (^W) */
	constexpr chtype _DLCHAR = 0x15;  /* Delete Line char (^U) */
	/* Struct for storing console registry keys, and for use with the
	undocumented WM_SETCONSOLEINFO message. Originally by James Brown,
	www.catch22.net. */
	#define A(x) ((chtype)x | A_ALTCHARSET)

	chtype acs_map[128] =
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

# undef A

	struct ConsoleInfo
	{
		ULONG    Length;
		COORD    ScreenBufferSize;
		COORD    WindowSize;
		ULONG    WindowPosX;
		ULONG    WindowPosY;

		COORD    FontSize;
		ULONG    FontFamily;
		ULONG    FontWeight;
		WCHAR    FaceName[32];

		ULONG    CursorSize;
		ULONG    FullScreen;
		ULONG    QuickEdit;
		ULONG    AutoPosition;
		ULONG    InsertMode;

		USHORT   ScreenColors;
		USHORT   PopupColors;
		ULONG    HistoryNoDup;
		ULONG    HistoryBufferSize;
		ULONG    NumberOfHistoryBuffers;

		COLORREF ColorTable[16];

		ULONG    CodePage;
		HWND     Hwnd;

		WCHAR    ConsoleTitle[0x100];
	};
	static ConsoleInfo console_info;
	static CONSOLE_SCREEN_BUFFER_INFO orig_scr;

	static CHAR_INFO *ci_save = NULL;
	static DWORD old_console_mode = 0;

	static INPUT_RECORD save_ip;
	//static MouseStatus old_mouse_status;
	static DWORD event_count = 0;
	static SHORT left_key;
	static int key_count = 0;
	static int save_press = 0;
	static Screen SP;


//constexpr chtype KEV = save_ip.Event.KeyEvent; 
//constexpr chtype MEV = save_ip.Event.MouseEvent; 


	/************************************************************************
	*    Table for key code translation of function keys in keypad mode    *
	*    These values are for strict IBM keyboard compatibles only         *
	************************************************************************/
	
	struct KPTAB
	{
		unsigned short normal;
		unsigned short shift;
		unsigned short control;
		unsigned short alt;
		unsigned short extended;
	};

	static KPTAB kptab[] =
	{
		{ 0,          0,         0,           0,          0 }, /* 0  */
		{ 0,          0,         0,           0,          0 }, /* 1   VK_LBUTTON */
		{ 0,          0,         0,           0,          0 }, /* 2   VK_RBUTTON */
		{ 0,          0,         0,           0,          0 }, /* 3   VK_CANCEL  */
		{ 0,          0,         0,           0,          0 }, /* 4   VK_MBUTTON */
		{ 0,          0,         0,           0,          0 }, /* 5   */
		{ 0,          0,         0,           0,          0 }, /* 6   */
		{ 0,          0,         0,           0,          0 }, /* 7   */
		{ 0x08,       0x08,      0x7F,        ALT_BKSP,   0 }, /* 8   VK_BACK    */
		{ 0x09,       KEY_BTAB,  CTL_TAB,     ALT_TAB,    999 }, /* 9   VK_TAB     */
		{ 0,          0,         0,           0,          0 }, /* 10  */
		{ 0,          0,         0,           0,          0 }, /* 11  */
		{ KEY_B2,     0x35,      CTL_PAD5,    ALT_PAD5,   0 }, /* 12  VK_CLEAR   */
		{ 0x0D,       0x0D,      CTL_ENTER,   ALT_ENTER,  1 }, /* 13  VK_RETURN  */
		{ 0,          0,         0,           0,          0 }, /* 14  */
		{ 0,          0,         0,           0,          0 }, /* 15  */
		{ 0,          0,         0,           0,          0 }, /* 16  VK_SHIFT   HANDLED SEPARATELY */
		{ 0,          0,         0,           0,          0 }, /* 17  VK_CONTROL HANDLED SEPARATELY */
		{ 0,          0,         0,           0,          0 }, /* 18  VK_MENU    HANDLED SEPARATELY */
		{ 0,          0,         0,           0,          0 }, /* 19  VK_PAUSE   */
		{ 0,          0,         0,           0,          0 }, /* 20  VK_CAPITAL HANDLED SEPARATELY */
		{ 0,          0,         0,           0,          0 }, /* 21  VK_HANGUL  */
		{ 0,          0,         0,           0,          0 }, /* 22  */
		{ 0,          0,         0,           0,          0 }, /* 23  VK_JUNJA   */
		{ 0,          0,         0,           0,          0 }, /* 24  VK_FINAL   */
		{ 0,          0,         0,           0,          0 }, /* 25  VK_HANJA   */
		{ 0,          0,         0,           0,          0 }, /* 26  */
		{ 0x1B,       0x1B,      0x1B,        ALT_ESC,    0 }, /* 27  VK_ESCAPE  */
		{ 0,          0,         0,           0,          0 }, /* 28  VK_CONVERT */
		{ 0,          0,         0,           0,          0 }, /* 29  VK_NONCONVERT */
		{ 0,          0,         0,           0,          0 }, /* 30  VK_ACCEPT  */
		{ 0,          0,         0,           0,          0 }, /* 31  VK_MODECHANGE */
		{ 0x20,       0x20,      0x20,        0x20,       0 }, /* 32  VK_SPACE   */
		{ KEY_A3,     0x39,      CTL_PAD9,    ALT_PAD9,   3 }, /* 33  VK_PRIOR   */
		{ KEY_C3,     0x33,      CTL_PAD3,    ALT_PAD3,   4 }, /* 34  VK_NEXT    */
		{ KEY_C1,     0x31,      CTL_PAD1,    ALT_PAD1,   5 }, /* 35  VK_END     */
		{ KEY_A1,     0x37,      CTL_PAD7,    ALT_PAD7,   6 }, /* 36  VK_HOME    */
		{ KEY_B1,     0x34,      CTL_PAD4,    ALT_PAD4,   7 }, /* 37  VK_LEFT    */
		{ KEY_A2,     0x38,      CTL_PAD8,    ALT_PAD8,   8 }, /* 38  VK_UP      */
		{ KEY_B3,     0x36,      CTL_PAD6,    ALT_PAD6,   9 }, /* 39  VK_RIGHT   */
		{ KEY_C2,     0x32,      CTL_PAD2,    ALT_PAD2,   10 }, /* 40  VK_DOWN    */
		{ 0,          0,         0,           0,          0 }, /* 41  VK_SELECT  */
		{ 0,          0,         0,           0,          0 }, /* 42  VK_PRINT   */
		{ 0,          0,         0,           0,          0 }, /* 43  VK_EXECUTE */
		{ 0,          0,         0,           0,          0 }, /* 44  VK_SNAPSHOT*/
		{ PAD0,       0x30,      CTL_PAD0,    ALT_PAD0,   11 }, /* 45  VK_INSERT  */
		{ PADSTOP,    0x2E,      CTL_PADSTOP, ALT_PADSTOP,12 }, /* 46  VK_DELETE  */
		{ 0,          0,         0,           0,          0 }, /* 47  VK_HELP    */
		{ 0x30,       0x29,      0,           ALT_0,      0 }, /* 48  */
		{ 0x31,       0x21,      0,           ALT_1,      0 }, /* 49  */
		{ 0x32,       0x40,      0,           ALT_2,      0 }, /* 50  */
		{ 0x33,       0x23,      0,           ALT_3,      0 }, /* 51  */
		{ 0x34,       0x24,      0,           ALT_4,      0 }, /* 52  */
		{ 0x35,       0x25,      0,           ALT_5,      0 }, /* 53  */
		{ 0x36,       0x5E,      0,           ALT_6,      0 }, /* 54  */
		{ 0x37,       0x26,      0,           ALT_7,      0 }, /* 55  */
		{ 0x38,       0x2A,      0,           ALT_8,      0 }, /* 56  */
		{ 0x39,       0x28,      0,           ALT_9,      0 }, /* 57  */
		{ 0,          0,         0,           0,          0 }, /* 58  */
		{ 0,          0,         0,           0,          0 }, /* 59  */
		{ 0,          0,         0,           0,          0 }, /* 60  */
		{ 0,          0,         0,           0,          0 }, /* 61  */
		{ 0,          0,         0,           0,          0 }, /* 62  */
		{ 0,          0,         0,           0,          0 }, /* 63  */
		{ 0,          0,         0,           0,          0 }, /* 64  */
		{ 0x61,       0x41,      0x01,        ALT_A,      0 }, /* 65  */
		{ 0x62,       0x42,      0x02,        ALT_B,      0 }, /* 66  */
		{ 0x63,       0x43,      0x03,        ALT_C,      0 }, /* 67  */
		{ 0x64,       0x44,      0x04,        ALT_D,      0 }, /* 68  */
		{ 0x65,       0x45,      0x05,        ALT_E,      0 }, /* 69  */
		{ 0x66,       0x46,      0x06,        ALT_F,      0 }, /* 70  */
		{ 0x67,       0x47,      0x07,        ALT_G,      0 }, /* 71  */
		{ 0x68,       0x48,      0x08,        ALT_H,      0 }, /* 72  */
		{ 0x69,       0x49,      0x09,        ALT_I,      0 }, /* 73  */
		{ 0x6A,       0x4A,      0x0A,        ALT_J,      0 }, /* 74  */
		{ 0x6B,       0x4B,      0x0B,        ALT_K,      0 }, /* 75  */
		{ 0x6C,       0x4C,      0x0C,        ALT_L,      0 }, /* 76  */
		{ 0x6D,       0x4D,      0x0D,        ALT_M,      0 }, /* 77  */
		{ 0x6E,       0x4E,      0x0E,        ALT_N,      0 }, /* 78  */
		{ 0x6F,       0x4F,      0x0F,        ALT_O,      0 }, /* 79  */
		{ 0x70,       0x50,      0x10,        ALT_P,      0 }, /* 80  */
		{ 0x71,       0x51,      0x11,        ALT_Q,      0 }, /* 81  */
		{ 0x72,       0x52,      0x12,        ALT_R,      0 }, /* 82  */
		{ 0x73,       0x53,      0x13,        ALT_S,      0 }, /* 83  */
		{ 0x74,       0x54,      0x14,        ALT_T,      0 }, /* 84  */
		{ 0x75,       0x55,      0x15,        ALT_U,      0 }, /* 85  */
		{ 0x76,       0x56,      0x16,        ALT_V,      0 }, /* 86  */
		{ 0x77,       0x57,      0x17,        ALT_W,      0 }, /* 87  */
		{ 0x78,       0x58,      0x18,        ALT_X,      0 }, /* 88  */
		{ 0x79,       0x59,      0x19,        ALT_Y,      0 }, /* 89  */
		{ 0x7A,       0x5A,      0x1A,        ALT_Z,      0 }, /* 90  */
		{ 0,          0,         0,           0,          0 }, /* 91  VK_LWIN    */
		{ 0,          0,         0,           0,          0 }, /* 92  VK_RWIN    */
		{ 0,          0,         0,           0,          0 }, /* 93  VK_APPS    */
		{ 0,          0,         0,           0,          0 }, /* 94  */
		{ 0,          0,         0,           0,          0 }, /* 95  */
		{ 0x30,       0,         CTL_PAD0,    ALT_PAD0,   0 }, /* 96  VK_NUMPAD0 */
		{ 0x31,       0,         CTL_PAD1,    ALT_PAD1,   0 }, /* 97  VK_NUMPAD1 */
		{ 0x32,       0,         CTL_PAD2,    ALT_PAD2,   0 }, /* 98  VK_NUMPAD2 */
		{ 0x33,       0,         CTL_PAD3,    ALT_PAD3,   0 }, /* 99  VK_NUMPAD3 */
		{ 0x34,       0,         CTL_PAD4,    ALT_PAD4,   0 }, /* 100 VK_NUMPAD4 */
		{ 0x35,       0,         CTL_PAD5,    ALT_PAD5,   0 }, /* 101 VK_NUMPAD5 */
		{ 0x36,       0,         CTL_PAD6,    ALT_PAD6,   0 }, /* 102 VK_NUMPAD6 */
		{ 0x37,       0,         CTL_PAD7,    ALT_PAD7,   0 }, /* 103 VK_NUMPAD7 */
		{ 0x38,       0,         CTL_PAD8,    ALT_PAD8,   0 }, /* 104 VK_NUMPAD8 */
		{ 0x39,       0,         CTL_PAD9,    ALT_PAD9,   0 }, /* 105 VK_NUMPAD9 */
		{ PADSTAR,   SHF_PADSTAR,CTL_PADSTAR, ALT_PADSTAR,999 }, /* 106 VK_MULTIPLY*/
		{ PADPLUS,   SHF_PADPLUS,CTL_PADPLUS, ALT_PADPLUS,999 }, /* 107 VK_ADD     */
		{ 0,          0,         0,           0,          0 }, /* 108 VK_SEPARATOR     */
		{ PADMINUS, SHF_PADMINUS,CTL_PADMINUS,ALT_PADMINUS,999 }, /* 109 VK_SUBTRACT*/
		{ 0x2E,       0,         CTL_PADSTOP, ALT_PADSTOP,0 }, /* 110 VK_DECIMAL */
		{ PADSLASH,  SHF_PADSLASH,CTL_PADSLASH,ALT_PADSLASH,2 }, /* 111 VK_DIVIDE  */
		{ KEY_F(1),   KEY_F(13), KEY_F(25),   KEY_F(37),  0 }, /* 112 VK_F1      */
		{ KEY_F(2),   KEY_F(14), KEY_F(26),   KEY_F(38),  0 }, /* 113 VK_F2      */
		{ KEY_F(3),   KEY_F(15), KEY_F(27),   KEY_F(39),  0 }, /* 114 VK_F3      */
		{ KEY_F(4),   KEY_F(16), KEY_F(28),   KEY_F(40),  0 }, /* 115 VK_F4      */
		{ KEY_F(5),   KEY_F(17), KEY_F(29),   KEY_F(41),  0 }, /* 116 VK_F5      */
		{ KEY_F(6),   KEY_F(18), KEY_F(30),   KEY_F(42),  0 }, /* 117 VK_F6      */
		{ KEY_F(7),   KEY_F(19), KEY_F(31),   KEY_F(43),  0 }, /* 118 VK_F7      */
		{ KEY_F(8),   KEY_F(20), KEY_F(32),   KEY_F(44),  0 }, /* 119 VK_F8      */
		{ KEY_F(9),   KEY_F(21), KEY_F(33),   KEY_F(45),  0 }, /* 120 VK_F9      */
		{ KEY_F(10),  KEY_F(22), KEY_F(34),   KEY_F(46),  0 }, /* 121 VK_F10     */
		{ KEY_F(11),  KEY_F(23), KEY_F(35),   KEY_F(47),  0 }, /* 122 VK_F11     */
		{ KEY_F(12),  KEY_F(24), KEY_F(36),   KEY_F(48),  0 }, /* 123 VK_F12     */

															   /* 124 through 218 */

		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0 },

		{ 0x5B,       0x7B,      0x1B,        ALT_LBRACKET,0 }, /* 219 */
		{ 0x5C,       0x7C,      0x1C,        ALT_BSLASH, 0 }, /* 220 */
		{ 0x5D,       0x7D,      0x1D,        ALT_RBRACKET,0 }, /* 221 */
		{ 0,          0,         0x27,        ALT_FQUOTE, 0 }, /* 222 */
		{ 0,          0,         0,           0,          0 }, /* 223 */
		{ 0,          0,         0,           0,          0 }, /* 224 */
		{ 0,          0,         0,           0,          0 }  /* 225 */
	};

	static KPTAB ext_kptab[] =
	{
		{ 0,          0,              0,              0, }, /* MUST BE EMPTY */
		{ PADENTER,   SHF_PADENTER,   CTL_PADENTER,   ALT_PADENTER }, /* 13 */
		{ PADSLASH,   SHF_PADSLASH,   CTL_PADSLASH,   ALT_PADSLASH }, /* 111 */
		{ KEY_PPAGE,  KEY_SPREVIOUS,  CTL_PGUP,       ALT_PGUP }, /* 33 */
		{ KEY_NPAGE,  KEY_SNEXT,      CTL_PGDN,       ALT_PGDN }, /* 34 */
		{ KEY_END,    KEY_SEND,       CTL_END,        ALT_END }, /* 35 */
		{ KEY_HOME,   KEY_SHOME,      CTL_HOME,       ALT_HOME }, /* 36 */
		{ KEY_LEFT,   KEY_SLEFT,      CTL_LEFT,       ALT_LEFT }, /* 37 */
		{ KEY_UP,     KEY_SUP,        CTL_UP,         ALT_UP }, /* 38 */
		{ KEY_RIGHT,  KEY_SRIGHT,     CTL_RIGHT,      ALT_RIGHT }, /* 39 */
		{ KEY_DOWN,   KEY_SDOWN,      CTL_DOWN,       ALT_DOWN }, /* 40 */
		{ KEY_IC,     KEY_SIC,        CTL_INS,        ALT_INS }, /* 45 */
		{ KEY_DC,     KEY_SDC,        CTL_DEL,        ALT_DEL }, /* 46 */
		{ PADSLASH,   SHF_PADSLASH,   CTL_PADSLASH,   ALT_PADSLASH }, /* 191 */
	};

	void Window::wnoutrefresh() {
		int begy, begx;     /* window's place on screen   */
		int i, j;

		if (_flags & (_PAD | _SUBPAD)) return;

		begy = _begy;
		begx = _begx;

		for (i = 0, j = begy; i < _maxy; i++, j++)
		{
			if (_firstch[i] != _NO_CHANGE)
			{
				chtype *src = _y[i];
				chtype *dest = curscr->_y[j] + begx;

				int first = _firstch[i]; /* first changed */
				int last = _lastch[i];   /* last changed */

											  /* ignore areas on the outside that are marked as changed,
											  but really aren't */

				while (first <= last && src[first] == dest[first])
					first++;

				while (last >= first && src[last] == dest[last])
					last--;

				/* if any have really changed... */

				if (first <= last)
				{
					memcpy(dest + first, src + first,
						(last - first + 1) * sizeof(chtype));

					first += begx;
					last += begx;

					if (first < curscr->_firstch[j] ||
						curscr->_firstch[j] == _NO_CHANGE)
						curscr->_firstch[j] = first;

					if (last > curscr->_lastch[j])
						curscr->_lastch[j] = last;
				}

				_firstch[i] = _NO_CHANGE;  /* updated now */
			}

			_lastch[i] = _NO_CHANGE;       /* updated now */
		}

		if (_clear) _clear = false;

		if (!_leaveit)
		{
			curscr->_cury =_cury + begy;
			curscr->_curx = _curx + begx;
		}

	}
	bool Window::wrefresh() {
		//PDC_LOG(("wrefresh() - called\n"));

		if (_flags & (_PAD | _SUBPAD)) return false;

		bool save_clear = _clear;

		if (this == curscr)
			curscr->_clear = true;
		else
			this->wnoutrefresh();

		if (save_clear && _maxy == SP.lines && _maxx == SP.cols)
			curscr._clear = true;

		return doupdate();
	}
	bool Window::wredrawln(int start, int num) {
	//	PDC_LOG(("wredrawln() - called: win=%p start=%d num=%d\n",
	//		win, start, num));

		if (start > _maxy || start + num > _maxy) return false;

		for (int i = start; i < start + num; i++)
		{
			_firstch[i] = 0;
			_lastch[i] = _maxx - 1;
		}

		return true;
	}
	bool Window::redrawwin() {
		//PDC_LOG(("redrawwin() - called: win=%p\n", win));

		return wredrawln(0, _maxy);
	}
	bool doupdate() {
		int y;
		bool clearall;
		if (!curscr) return false;

		if (isendwin())         /* coming back after endwin() called */
		{
			priv::reset_prog_mode();
			clearall = true;
			SP.alive = true;   /* so isendwin() result is correct */
		}
		else
			clearall = curscr->_clear;

		for (y = 0; y < SP->lines; y++)
		{
		//	PDC_LOG(("doupdate() - Transforming line %d of %d: %s\n",
		//		y, SP->lines, (curscr->_firstch[y] != _NO_CHANGE) ?
		//		"Yes" : "No"));

			if (clearall || curscr->_firstch[y] != _NO_CHANGE)
			{
				int first, last;

				chtype *src = curscr->_y[y];
				chtype *dest = pdc_lastscr->_y[y];

				if (clearall)
				{
					first = 0;
					last = COLS - 1;
				}
				else
				{
					first = curscr->_firstch[y];
					last = curscr->_lastch[y];
				}

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
						priv::transform_line(y, first, len, src + first);
						std::memcpy(dest + first, src + first, len * sizeof(chtype));
						first += len;
					}

					/* skip over runs of unchanged cells */

					while (first <= last && src[first] == dest[first])
						first++;
				}

				curscr->_firstch[y] = _NO_CHANGE;
				curscr->_lastch[y] = _NO_CHANGE;
			}
		}

		curscr->_clear = FALSE;

		if (SP->visibility)
			priv::gotoyx(curscr->_cury, curscr->_curx);

		SP->cursrow = curscr->_cury;
		SP->curscol = curscr->_curx;

		return true;
	}
	bool refresh()
	{
		//PDC_LOG(("refresh() - called\n"));

		//return stdscr->wrefresh();
	}
	namespace priv {
	
		
		static HWND _find_console_handle(void)
		{
			TCHAR orgtitle[1024], temptitle[1024];
			HWND wnd;

			GetConsoleTitle(orgtitle, 1024);

			wsprintf(temptitle, TEXT("%d/%d"), GetTickCount(), GetCurrentProcessId());
			SetConsoleTitle(temptitle);

			Sleep(40);

			wnd = FindWindow(NULL, temptitle);

			SetConsoleTitle(orgtitle);

			return wnd;
		}

		/* Undocumented console message */

constexpr chtype WM_SETCONSOLEINFO = (WM_USER + 201); 

		/* Wrapper around WM_SETCONSOLEINFO. We need to create the necessary
		section (file-mapping) object in the context of the process which
		owns the console, before posting the message. Originally by JB. */

		static void _set_console_info(void)
		{
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			CONSOLE_CURSOR_INFO cci;
			DWORD dwConsoleOwnerPid;
			HANDLE hProcess;
			HANDLE hSection, hDupSection;
			PVOID ptrView;

			/* Each-time initialization for console_info */

			GetConsoleCursorInfo(pdc_con_out, &cci);
			console_info.CursorSize = cci.dwSize;

			GetConsoleScreenBufferInfo(pdc_con_out, &csbi);
			console_info.ScreenBufferSize = csbi.dwSize;

			console_info.WindowSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			console_info.WindowSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

			console_info.WindowPosX = csbi.srWindow.Left;
			console_info.WindowPosY = csbi.srWindow.Top;

			/* Open the process which "owns" the console */

			GetWindowThreadProcessId(console_info.Hwnd, &dwConsoleOwnerPid);

			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwConsoleOwnerPid);

			/* Create a SECTION object backed by page-file, then map a view of
			this section into the owner process so we can write the contents
			of the CONSOLE_INFO buffer into it */

			hSection = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE,
				0, sizeof(console_info), 0);

			/* Copy our console structure into the section-object */

			ptrView = MapViewOfFile(hSection, FILE_MAP_WRITE | FILE_MAP_READ,
				0, 0, sizeof(console_info));

			memcpy(ptrView, &console_info, sizeof(console_info));

			UnmapViewOfFile(ptrView);

			/* Map the memory into owner process */

			DuplicateHandle(GetCurrentProcess(), hSection, hProcess, &hDupSection,
				0, FALSE, DUPLICATE_SAME_ACCESS);

			/* Send console window the "update" message */

			SendMessage(console_info.Hwnd, WM_SETCONSOLEINFO, (WPARAM)hDupSection, 0);

			CloseHandle(hSection);
			CloseHandle(hProcess);
		}
		/* One-time initialization for console_info -- color table and font info
		from the registry; other values from functions. */

		static void _init_console_info(void)
		{
			DWORD scrnmode, len;
			HKEY reghnd;
			int i;

			console_info.Hwnd = _find_console_handle();
			console_info.Length = sizeof(console_info);

			GetConsoleMode(pdc_con_in, &scrnmode);
			console_info.QuickEdit = !!(scrnmode & 0x0040);
			console_info.InsertMode = !!(scrnmode & 0x0020);

			console_info.FullScreen = FALSE;
			console_info.AutoPosition = 0x10000;
			console_info.ScreenColors = SP.orig_back << 4 | SP.orig_fore;
			console_info.PopupColors = 0xf5;

			console_info.HistoryNoDup = FALSE;
			console_info.HistoryBufferSize = 50;
			console_info.NumberOfHistoryBuffers = 4;

			console_info.CodePage = GetConsoleOutputCP();

			RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Console"), 0,
				KEY_QUERY_VALUE, &reghnd);

			len = sizeof(DWORD);

			/* Default color table */
			char tname[] = "ColorTable\0\0\0";
			for (i = 0; i < 16; i++)
			{
				tname[10] = '0' + (i / 10);
				tname[12] = '0' + (i % 10);
				RegQueryValueExA(reghnd, tname, NULL, NULL,
					(LPBYTE)(&(console_info.ColorTable[i])), &len);
			}

			/* Font info */

			RegQueryValueEx(reghnd, TEXT("FontSize"), NULL, NULL,
				(LPBYTE)(&console_info.FontSize), &len);
			RegQueryValueEx(reghnd, TEXT("FontFamily"), NULL, NULL,
				(LPBYTE)(&console_info.FontFamily), &len);
			RegQueryValueEx(reghnd, TEXT("FontWeight"), NULL, NULL,
				(LPBYTE)(&console_info.FontWeight), &len);

			len = sizeof(WCHAR) * 32;
			RegQueryValueExW(reghnd, L"FaceName", NULL, NULL,
				(LPBYTE)(console_info.FaceName), &len);

			RegCloseKey(reghnd);
		}
		bool check_key() {
			if (key_count > 0) return true;
			DWORD event_count;
			GetNumberOfConsoleInputEvents(pdc_con_in, &event_count);
			return event_count != 0;
		}

		bool scr_open() {
			COORD bufsize, origin;
			SMALL_RECT rect;
			const char *str;
			CONSOLE_SCREEN_BUFFER_INFO csbi;

			
			pdc_atrtab.assign(PDC_COLOR_PAIRS * PDC_OFFSET, 0); 

			for (short i = 0; i < 16; i++) curstoreal[realtocurs[i]] = i;
			pdc_con_out = GetStdHandle(STD_OUTPUT_HANDLE);
			pdc_con_in = GetStdHandle(STD_INPUT_HANDLE);

			/* preserve QuickEdit Mode setting for use in PDC_mouse_set() when
			the mouse is not enabled -- other console input settings are
			cleared */

			pdc_quick_edit = old_console_mode & 0x0040;


			GetConsoleScreenBufferInfo(pdc_con_out, &csbi);
			GetConsoleScreenBufferInfo(pdc_con_out, &orig_scr);
			GetConsoleMode(pdc_con_in, &old_console_mode);
			SP.lines = get_rows();
			SP.cols = get_columns();
		
			SP.mouse_wait = PDC_CLICK_PERIOD;
			SP.audible = true;
			if (SP.lines < 2 || SP.lines > csbi.dwMaximumWindowSize.Y)
			{
				fprintf(stderr, "LINES value must be >= 2 and <= %d: got %d\n",
					csbi.dwMaximumWindowSize.Y, SP.lines);

				return false;
			}
			if (SP.cols < 2 || SP.cols > csbi.dwMaximumWindowSize.X)
			{
				fprintf(stderr, "COLS value must be >= 2 and <= %d: got %d\n",
					csbi.dwMaximumWindowSize.X, SP.cols);

				return false;
			}
			SP.orig_fore = csbi.wAttributes & 0x0f;
			SP.orig_back = (csbi.wAttributes & 0xf0) >> 4;
			SP.orig_attr = true;
			reset_prog_mode();

			SP.mono = false;

			return true;
		}
		void flushinp()
		{

			FlushConsoleInputBuffer(pdc_con_in);
		}
		/* Calls SetConsoleWindowInfo with the given parameters, but fits them
		if a scoll bar shrinks the maximum possible value. The rectangle
		must at least fit in a half-sized window. */

		static bool _fit_console_window(HANDLE con_out, const SMALL_RECT &rect)
		{
			SMALL_RECT run;
			SHORT mx, my;

			if (SetConsoleWindowInfo(con_out, TRUE, &rect)) return true;

			run = rect;
			run.Right /= 2;
			run.Bottom /= 2;

			mx = run.Right;
			my = run.Bottom;

			if (!SetConsoleWindowInfo(con_out, TRUE, &run))
				return false;

			for (run.Right = rect.Right; run.Right >= mx; run.Right--)
				if (SetConsoleWindowInfo(con_out, TRUE, &run))
					break;

			if (run.Right < mx)
				return false;

			for (run.Bottom = rect.Bottom; run.Bottom >= my; run.Bottom--)
				if (SetConsoleWindowInfo(con_out, TRUE, &run))
					return true;

			return false;
		}
		bool resize_screen(int nlines, int ncols) {
			SMALL_RECT rect;
			COORD size, max;

			if (nlines < 2 || ncols < 2) return false;

			max = GetLargestConsoleWindowSize(pdc_con_out);

			rect.Left = rect.Top = 0;
			rect.Right = ncols - 1;

			if (rect.Right > max.X)
				rect.Right = max.X;

			rect.Bottom = nlines - 1;

			if (rect.Bottom > max.Y)
				rect.Bottom = max.Y;

			size.X = rect.Right + 1;
			size.Y = rect.Bottom + 1;

			_fit_console_window(pdc_con_out, rect);
			SetConsoleScreenBufferSize(pdc_con_out, size);
			_fit_console_window(pdc_con_out, rect);
			SetConsoleScreenBufferSize(pdc_con_out, size);
			SetConsoleActiveScreenBuffer(pdc_con_out);

			return true;
		}
		void reset_prog_mode() {
			COORD bufsize;
			SMALL_RECT rect;

			bufsize.X = orig_scr.srWindow.Right - orig_scr.srWindow.Left + 1;
			bufsize.Y = orig_scr.srWindow.Bottom - orig_scr.srWindow.Top + 1;

			rect.Top = rect.Left = 0;
			rect.Bottom = bufsize.Y - 1;
			rect.Right = bufsize.X - 1;

			SetConsoleScreenBufferSize(pdc_con_out, bufsize);
			SetConsoleWindowInfo(pdc_con_out, TRUE, &rect);
			SetConsoleScreenBufferSize(pdc_con_out, bufsize);
			SetConsoleActiveScreenBuffer(pdc_con_out);
			mouse_set();
		}
		void mouse_set(void)
		{
			/* If turning on mouse input: Set ENABLE_MOUSE_INPUT, and clear
			all other flags, including the extended flags;
			If turning off the mouse: Set QuickEdit Mode to the status it
			had on startup, and clear all other flags */

			SetConsoleMode(pdc_con_in, (SP._trap_mbe ?	(ENABLE_MOUSE_INPUT | 0x0080) : (pdc_quick_edit | 0x0080)));

		//	memset(&old_mouse_status, 0, sizeof(old_mouse_status));

		}
		void reset_shell_mode()
		{
			SetConsoleScreenBufferSize(pdc_con_out, orig_scr.dwSize);
			SetConsoleWindowInfo(pdc_con_out, TRUE, &orig_scr.srWindow);
			SetConsoleScreenBufferSize(pdc_con_out, orig_scr.dwSize);
			SetConsoleWindowInfo(pdc_con_out, TRUE, &orig_scr.srWindow);
			SetConsoleActiveScreenBuffer(pdc_con_out);

			SetConsoleMode(pdc_con_in, old_console_mode);
		}
		void init_pair(short pair, short fg, short bg) {
			unsigned char att, temp_bg;
			chtype i;

			fg = curstoreal[fg];
			bg = curstoreal[bg];

			for (i = 0; i < PDC_OFFSET; i++)
			{
				att = fg | (bg << 4);

				if (i & (A_REVERSE >> PDC_ATTR_SHIFT))
					att = bg | (fg << 4);
				if (i & (A_UNDERLINE >> PDC_ATTR_SHIFT))
					att = 1;
				if (i & (A_INVIS >> PDC_ATTR_SHIFT))
				{
					temp_bg = att >> 4;
					att = temp_bg << 4 | temp_bg;
				}
				if (i & (A_BOLD >> PDC_ATTR_SHIFT))
					att |= 8;
				if (i & (A_BLINK >> PDC_ATTR_SHIFT))
					att |= 128;

				pdc_atrtab[pair * PDC_OFFSET + i] = att;
			}
		}
		void pair_content(short pair, short *fg, short *bg) {
			*fg = realtocurs[pdc_atrtab[pair * PDC_OFFSET] & 0x0F];
			*bg = realtocurs[(pdc_atrtab[pair * PDC_OFFSET] & 0xF0) >> 4];
		}
		void color_content(short color, short *red, short *green, short *blue) {
			DWORD col;

			if (!console_info.Hwnd)
				_init_console_info();

			col = console_info.ColorTable[curstoreal[color]];

			*red = DIVROUND(GetRValue(col) * 1000, 255);
			*green = DIVROUND(GetGValue(col) * 1000, 255);
			*blue = DIVROUND(GetBValue(col) * 1000, 255);

		}
		void init_color(short color, short red, short green, short blue) {
			if (!console_info.Hwnd)
				_init_console_info();

			console_info.ColorTable[curstoreal[color]] =
				RGB(DIVROUND(red * 255, 1000),
					DIVROUND(green * 255, 1000),
					DIVROUND(blue * 255, 1000));

			_set_console_info();

		}
		/* return number of screen rows */

		int get_rows()
		{
			CONSOLE_SCREEN_BUFFER_INFO scr;
			GetConsoleScreenBufferInfo(pdc_con_out, &scr);
			return scr.srWindow.Bottom - scr.srWindow.Top + 1;
		}

		/* return number of buffer rows */

		int get_buffer_rows()
		{
			CONSOLE_SCREEN_BUFFER_INFO scr;
			GetConsoleScreenBufferInfo(pdc_con_out, &scr);
			return scr.dwSize.Y;
		}

		/* return width of screen/viewport */

		int get_columns()
		{
			CONSOLE_SCREEN_BUFFER_INFO scr;
			GetConsoleScreenBufferInfo(pdc_con_out, &scr);
			return scr.srWindow.Right - scr.srWindow.Left + 1;
		}
		void gotoyx(int row, int col) {
			const COORD coord = { row, col };
			SetConsoleCursorPosition(pdc_con_out, coord);
		}
		void transform_line(int lineno, int x, int len, const chtype *srcp) {
			CHAR_INFO ci[512];
			COORD bufPos = { 0, 0 };
			COORD bufSize = { len, 1 };
			COORD bufSize, bufPos;
			SMALL_RECT sr = { x, lineno, x + len - 1, lineno };
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

			WriteConsoleOutput(pdc_con_out, ci, bufSize, bufPos, &sr);
		}
	};
};
