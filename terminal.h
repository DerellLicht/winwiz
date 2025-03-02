bool term_notify(HWND hwnd, LPARAM lParam);
void setup_terminal_window(HWND hwnd, uint StatusBarHeight, uint bottom_ref_control);
WNDPROC term_lview_subclass(LONG TermSubclassProc);
void term_set_font(char *font_name, uint fsize, uint attr);

void term_set_attr(uint fgnd, uint bgnd);
void term_put(char *term_str);
void term_append(char *term_str);
void term_replace(char *term_str);
uint term_get_columns(void);
uint term_get_rows(void);
void term_copy_selected_rows(void);
void term_clear_message_area(void);

