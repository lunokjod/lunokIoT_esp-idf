#ifndef ___LUNOKIOT__ANSI__HELPER__
#define ___LUNOKIOT__ANSI__HELPER__

//@TODO incomplete list, add when used
#define TERM_QUERY "\033[c"
#define TERM_QUERY_DEVICE_STATUS "\033[5n"
#define TERM_QUERY_CURSOR_POSITION "\033[6n"
#define TERM_SAVE_CURSOR_POSITION "\033[s"
#define TERM_RESTORE_CURSOR_POSITION "\033[u"
#define TERM_SET_CURSOR_POSITION "\033[%d;%dH"
#define TERM_HIDE_CURSOR "\033[?25l"
#define TERM_SHOW_CURSOR "\033[?25h"

#define TERM_RESET "\033[m"

#define VT_ERASE "\033[2J"
#define VT_CURSOR_HOME "\033[0;0H"

#define TERM_BOLD "\033[1m"
#define TERM_REMOVE_BOLD "\033[22m"

#define TERM_FG_BLACK "\033[30m"
#define TERM_FG_RED "\033[31m"
#define TERM_FG_GREEN "\033[32m"
#define TERM_FG_WHITE "\033[37m"
#define TERM_FG_GREY "\033[90m"
#define TERM_FG_YELLOW "\033[93m"

#define TERM_BG_BLUE "\033[44m"
#define TERM_BG_BLACK "\033[40m"
#define TERM_BG_GRAY "\033[100m"

#endif // ___LUNOKIOT__ANSI__HELPER__