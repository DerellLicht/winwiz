//****************************************************************************
//  Copyright (c) 1985-2025  Daniel D Miller
//  winwiz.exe - Win32 version of Wizard's Castle
//****************************************************************************
//  WIZARD'S CASTLE GAME FROM JULY/AUGUST 1980                       
//  ISSUE OF RECREATIONAL COMPUTING MAGAZINE.                        
//  WRITTEN FOR EXIDY SORCERER BY JOSEPH R. POWER                    
//                                                                   
//  FALL QUARTER, 1985:                                              
//  REWRITTEN FOR IBM PC USING GWBASIC                               
//  BY DANIEL D. (derelict) MILLER                                   
//  CALIFORNIA POLYTECHNIC STATE UNIVERSITY                          
//  SAN LUIS OBISPO, CA.                                             
//                                                                   
//  Summer, 1986:                                                    
//  Converted to Turbo Pascal by the same derelict                   
//  Last Pascal revision: October 7, 1986                            
//                                                                   
//  Fall 2004                                                        
//  Converted to Win32, compiled with either MinGw or VC++6.         
//  Also, moved revision history from here to history.winwiz.txt     
//                                                                   
//  January 2011                                                     
//  Converted to dialog-box application, using my terminal object    
//  based on the Virtual ListView control                            
//****************************************************************************

#include <windows.h>
#include <stdio.h>   //  vsprintf, sprintf, which supports %f
#include <time.h>
#include <tchar.h>

#ifdef _lint
//  for some reason, lint requires stdlib.h in order for this to be defined.
//  yet g++ itself does not.
/* RAND_MAX is the maximum value that may be returned by rand.
 * The minimum is zero.  */
#define RAND_MAX  0x7FFF
#endif

#include "resource.h"
#include "version.h"
#include "common.h"
#include "commonw.h"
#include "statbar.h"
#include "cterminal.h"  //  MAX_TERM_CHARS
#include "terminal.h" 
#include "winmsgs.h"
#include "wizard.h"
#include "keywin32.h"
#include "tooltips.h"
#include "gdiplus_setup.h"

static TCHAR const * const Version = _T("Wizard's Castle, Version " VerNum " ") ;   //lint !e707

static HINSTANCE g_hinst = 0;

static HWND hwndMain ;

//lint -esym(714, dbg_flags)
//lint -esym(759, dbg_flags)
//lint -esym(765, dbg_flags)
uint dbg_flags = 0
   // | DBG_WINMSGS
   ;

static uint main_timer_id = 0 ;

uint cxClient = 0 ;
uint cyClient = 0 ;

static CStatusBar *MainStatusBar = NULL;

//  This does not appear to be required for anything
// static bool redraw_in_progress = false ;

//  This prevents map from being redrawn until program init is completed
// bool prog_init_done = false ;

//*******************************************************************
static void toggle_debug_flag(void)
{
   dbg_flags ^= DBG_WINMSGS ;
   TCHAR msg[80] ;
   _stprintf(msg, _T("debug flag is %s"), (dbg_flags & DBG_WINMSGS) ? _T("ENABLED") : _T("disabled"));
   infoout(msg);
}
            
//*************************************************************
static void debug_force_runestaff(void)
{
   player.has_runestaff = true ;
   put_color_msg(TERM_RUNESTAFF, _T("GREAT ZOT!! You've found the RUNESTAFF!!"));
   show_treasures() ;
}

//*******************************************************************
void show_help_menu(void)
{
   queryout(_T("Terminal keyboard shortcuts")) ;
   infoout(_T("Alt-b = Show About dialog")) ;
   infoout(_T("Alt-h = Show Help dialog")) ;
   infoout(_T("Alt-q = Close this program")) ;
   termout(_T("Alt-d = Debug; Toggle debug flag")) ;
   termout(_T("Alt-j = Debug: force runestaff")) ;
   termout(_T("Alt-k = Debug: show game states")) ;
}

//*******************************************************************
void status_message(TCHAR *msgstr)
{
   MainStatusBar->show_message(msgstr);
}

//*******************************************************************
void status_message(uint idx, TCHAR *msgstr)
{
   MainStatusBar->show_message(idx, msgstr);
}

//****************************************************************************
//lint -esym(756, attrib_table_t)
static attrib_table_t term_atable[NUM_TERM_ATTR_ENTRIES] = {
   { WIN_CYAN,    WIN_BLACK },    // TERM_NORMAL 
   { WIN_BCYAN,   WIN_GREY },     // TERM_INFO
   { WIN_YELLOW,  WIN_BLUE },     // TERM_QUERY
   { WIN_RED,     WIN_BLACK },    // TERM_PLAYER_HIT
   { WIN_BLUE,    WIN_BLACK },   // TERM_MONSTER_HIT
   { WIN_GREY,    WIN_BLACK },   // TERM_RUNESTAFF
   { WIN_BBLUE,   WIN_BLACK },   // TERM_DEATH
   { WIN_GREEN,   WIN_BLACK }    // TERM_ATMOSPHERE
} ;

//****************************************************************************
static void set_local_terminal_colors(void)
{
   COLORREF std_bgnd = GetSysColor(COLOR_WINDOW) ;
   term_atable[TERM_NORMAL].fgnd = GetSysColor(COLOR_WINDOWTEXT) ;
   term_atable[TERM_NORMAL].bgnd = std_bgnd ;

   //  set standard background for other color sets which use it
   term_atable[TERM_PLAYER_HIT].bgnd = std_bgnd ;
   term_atable[TERM_MONSTER_HIT].bgnd = std_bgnd ;
   term_atable[TERM_RUNESTAFF].bgnd = std_bgnd ;
   term_atable[TERM_DEATH].bgnd = std_bgnd ;
   term_atable[TERM_ATMOSPHERE].bgnd = std_bgnd ;
}

//********************************************************************
void set_term_attr_default(void)
{
   term_set_attr(term_atable[TERM_NORMAL].fgnd, term_atable[TERM_NORMAL].bgnd) ;
}

//********************************************************************
//lint -esym(714, set_term_attr, termout, put_color_term_msg)
//lint -esym(759, set_term_attr, termout, put_color_term_msg)
//lint -esym(765, set_term_attr, termout, put_color_term_msg)
void set_term_attr(uint atidx)
{
   if (atidx >= NUM_TERM_ATTR_ENTRIES) {
      syslog(_T("set_term_attr: invalid index %u\n"), atidx) ;
      return ;
   }
   term_set_attr(term_atable[atidx].fgnd, term_atable[atidx].bgnd) ;
}

//********************************************************************
//  This outputs to terminal in default colors
//********************************************************************
int termout(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   set_term_attr(TERM_NORMAL);
   term_put(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
int term_append(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // syslog("ecterm attribs: fgnd=%06X, bgnd=%06X\n", 
   //    term->term_fgnd, term->term_bgnd) ;
   // myTerminal->set_term_attr_default() ;
   // myTerminal->put(consoleBuffer);
   set_term_attr(TERM_NORMAL) ;
   term_append(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
int wterm_replace(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // syslog("ecterm attribs: fgnd=%06X, bgnd=%06X\n", 
   //    term->term_fgnd, term->term_bgnd) ;
   // myTerminal->set_term_attr_default() ;
   // myTerminal->put(consoleBuffer);
   set_term_attr(TERM_NORMAL) ;
   term_replace(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
int queryout(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // syslog("ecterm attribs: fgnd=%06X, bgnd=%06X\n", 
   //    term->term_fgnd, term->term_bgnd) ;
   // myTerminal->set_term_attr(WIN_YELLOW, WIN_BLUE) ;
   set_term_attr(TERM_QUERY) ;
   term_put(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
int infoout(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // syslog("ecterm attribs: fgnd=%06X, bgnd=%06X\n", 
   //    term->term_fgnd, term->term_bgnd) ;
   set_term_attr(TERM_INFO) ;
   term_put(consoleBuffer);
   va_end(al);
   return 1;
}

//*******************************************************************
//  this uses default font and color
//*******************************************************************
void put_message(TCHAR *msgstr)
{
   termout(msgstr) ;
}

//********************************************************************
//  this function is only used for displaying RuneStaff and/or
//  Orb Of Zot, in "treasures" listing
//********************************************************************
int put_message(COLORREF attr, const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   term_set_attr(attr, term_atable[TERM_NORMAL].bgnd) ;
   term_put(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
//  this function is only used for displaying RuneStaff and/or
//  Orb Of Zot, in "treasures" listing
//********************************************************************
int put_message(COLORREF fgnd, COLORREF bgnd, const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   term_set_attr(fgnd, bgnd) ;
   term_put(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
//  this *cannot* be called with a color attribute;
//  it must be called with an index into term_atable[] !!
//********************************************************************
int put_color_msg(uint idx, const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[MAX_TERM_CHARS + 1];
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   set_term_attr(idx) ;
   term_put(consoleBuffer);
   va_end(al);
   return 1;
}

//*************************************************************
//  Notes stolen from the Linux man page for rand():
//  
// In Numerical Recipes in C: The Art of Scientific Computing 
//  (William H. Press, Brian P. Flannery, Saul A. Teukolsky, 
//   William T. Vetterling; New York: Cambridge University Press,
//   1990 (1st ed, p. 207)), the following comments are made: 
// 
// "If you want to generate a random integer between 1 and 10, 
//  you should always do it by 
// 
//    j=1+(int) (10.0*rand()/(RAND_MAX+1.0)); 
// 
// and never by anything resembling 
// 
//    j=1+((int) (1000000.0*rand()) % 10); 
// 
// (which uses lower-order bits)." 
//*************************************************************
//  return a random number between 0-(Q-1)
//*************************************************************
unsigned random(unsigned Q)
{
   unsigned j = (unsigned) ((Q * rand2()) / (RAND_MAX+1)); 
   return j;
}         

//******************************************************************
// Subclass procedure for the Terminal Virtual ListView control
// http://msdn.microsoft.com/en-us/library/ms633570%28VS.85%29.aspx#subclassing_window
//******************************************************************
//  mon.support.cpp
static WNDPROC wpOrigTermProc; 
uint key_mask = 0 ;

static LRESULT APIENTRY TermSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
   char inchr ;

   if (uMsg == WM_GETDLGCODE) 
      return DLGC_WANTALLKEYS; 
 
   //***************************************************
   //  debug: log all windows messages
   //***************************************************
   if (dbg_flags & DBG_WINMSGS) {
      switch (uMsg) {
      //  list messages to be ignored
      case WM_KEYUP:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      case WM_CHAR:
      case WM_NCHITTEST:
      case WM_SETCURSOR:
      case WM_MOUSEMOVE:
      case WM_NOTIFY:
         break;
      default:
         syslog(_T("SubC [%s]\n"), lookup_winmsg_name(uMsg)) ;
         break;
      }
   }
   switch (uMsg) {
   case WM_KEYUP:
   case WM_SYSKEYUP:
      // ShowKey (hwnd, 0, "WM_KEYUP", wParam, lParam) ;
      switch (wParam) {
      case VK_SHIFT:   
         key_mask &= ~kShift;
         return 0;
      case VK_CONTROL: 
         key_mask &= ~kCtrl;
         return 0;
      case VK_MENU:    
         key_mask &= ~kAlt;
         return 0;
      default:
         break;
      }
      return 0;

   case WM_KEYDOWN:
   case WM_SYSKEYDOWN:
      // syslog(_T("WM_KEYDOWN: WPARAM=0x%04X, LPARAM=%u\n"), wParam, lParam) ;
      // inchr = (char) wParam ;
      // if (inchr == CtrlC) {
      //    // term_info_p tiSelf = find_term_from_hwnd(hwnd) ;
      //    copy_selected_rows(term) ;
      //    return 0;
      // }
      switch (wParam) {
      // ShowKey (hwnd, 0, "WM_KEYDOWN", wParam, lParam) ;
      case VK_SHIFT:   key_mask |= kShift;  return 0 ;
      case VK_CONTROL: key_mask |= kCtrl;  return 0 ;
      case VK_MENU:    key_mask |= kAlt;  return 0 ;
      // else {
      //    wParam |= key_mask;
      //    process_keystroke (hwnd, wParam);
      // }
      case VK_LEFT:
      case VK_UP:
      case VK_RIGHT:
      case VK_DOWN:
         // syslog(_T("WM_KEYDOWN: process_keystroke\n")) ;
         process_keystroke (hwnd, wParam) ;
         return 0;
      default:
         break;
      }
      break;

   // case WM_KEYUP:
   //    // syslog("WM_UP: [0x%X]\n", wParam) ;
   //    break;

   case WM_CHAR:
      // syslog("WM_CHAR: WPARAM=0x%04X, LPARAM=%u\n", wParam, lParam) ;
      inchr = (char) wParam ;
      if (inchr == CtrlC) {
         // term_info_p tiSelf = find_term_from_hwnd(hwnd) ;
         term_copy_selected_rows();
         return 0;
      }
      // syslog("WM_CHAR: process_keystroke [0x%X]\n", wParam) ;
      process_keystroke (hwnd, wParam);
      return 0;
   }  //lint !e744

   return CallWindowProc(wpOrigTermProc, hwnd, uMsg, wParam, lParam); 
}

//***********************************************************************
static uint screen_width  = 0 ;
static uint screen_height = 0 ;

static void ww_get_monitor_dimens(HWND hwnd)
{
   HMONITOR currentMonitor;      // Handle to monitor where fullscreen should go
   MONITORINFO mi;               // Info of that monitor
   currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
   mi.cbSize = sizeof(MONITORINFO);
   if (GetMonitorInfo(currentMonitor, &mi) != FALSE) {
      screen_width  = mi.rcMonitor.right  - mi.rcMonitor.left ;
      screen_height = mi.rcMonitor.bottom - mi.rcMonitor.top ;
   }
   // curr_dpi = GetScreenDPI() ;
}

//***********************************************************************
static void center_window(void)
{
   ww_get_monitor_dimens(hwndMain);
   
   RECT myRect ;
   GetWindowRect(hwndMain, &myRect) ;
   // GetClientRect(hwnd, &myRect) ;
   uint dialog_width = (myRect.right - myRect.left) ;
   uint dialog_height = (myRect.bottom - myRect.top) ;

   uint x0 = (screen_width  - dialog_width ) / 2 ;
   uint y0 = (screen_height - dialog_height) / 2 ;

   SetWindowPos(hwndMain, HWND_TOP, x0, y0, 0, 0, SWP_NOSIZE) ;
}

//***********************************************************************
static tooltip_data_t main_tooltips[] = {
{ IDC_T0,    _T(" the Ruby Red "  ) },  
{ IDC_T1,    _T(" the Norn Stone ") },  
{ IDC_T2,    _T(" the Pale Pearl ") },  
{ IDC_T3,    _T(" the Opal Eye "  ) },  
{ IDC_RS,    _T(" the RuneStaff " ) },
{ IDC_T4,    _T(" the Green Gem " ) },  
{ IDC_T5,    _T(" the Blue Flame ") },  
{ IDC_T6,    _T(" the Palantir "  ) },  
{ IDC_T7,    _T(" the Silmaril "  ) },  
{ IDC_OZ,    _T(" the Orb of Zot ") },
{ 0, NULL }} ;

//***********************************************************************
static void do_init_dialog(HWND hwnd)
{
   TCHAR msgstr[81] ;
   // hwndTopLevel = hwnd ;   //  do I need this?
   _stprintf(msgstr, _T("%s"), Version) ;
   SetWindowText(hwnd, msgstr) ;

   SetClassLong(hwnd, GCL_HICON,   (LONG) LoadIcon(g_hinst, (LPCTSTR)WINWIZICO));
   SetClassLong(hwnd, GCL_HICONSM, (LONG) LoadIcon(g_hinst, (LPCTSTR)WINWIZICO));

   hwndMain = hwnd ;

   set_up_working_spaces(hwnd) ; //  do this *before* tooltips !!
   //***************************************************************************
   //  add tooltips and bitmaps
   //***************************************************************************
   create_and_add_tooltips(hwnd, 150, 100, 10000, main_tooltips);

   if (dbg_flags & DBG_WINMSGS) {
      term_set_debug(true);
   }
   // RECT rWindow;
   // unsigned stTop ;
   RECT myRect ;
   // GetWindowRect(hwnd, &myRect) ;
   GetClientRect(hwnd, &myRect) ;
   cxClient = (myRect.right - myRect.left) ;
   cyClient = (myRect.bottom - myRect.top) ;

   center_window() ;
   //****************************************************************
   //  create/configure status bar
   //****************************************************************
   MainStatusBar = new CStatusBar(hwnd) ;
   MainStatusBar->MoveToBottom(cxClient, cyClient) ;
   //  re-position status-bar parts
   {
   int sbparts[3];
   sbparts[0] = (int) (6 * cxClient / 10) ;
   sbparts[1] = (int) (8 * cxClient / 10) ;
   sbparts[2] = -1;
   MainStatusBar->SetParts(3, &sbparts[0]);
   }
   
   //****************************************************************
   //  create/configure terminal
   //****************************************************************
   setup_terminal_window(hwnd, MainStatusBar->height(), IDC_MAP_AREA, IDC_TERMINAL);
   set_local_terminal_colors() ;
    
   // Subclass the terminal ListView
   // wpOrigTermProc = (WNDPROC) SetWindowLongPtr(term->hwndSelf, GWL_WNDPROC, (LONG) TermSubclassProc); 
   wpOrigTermProc = term_lview_subclass((LONG) TermSubclassProc); 
      
   // SetClassLong(this_port->cpterm->hwndSelf, GCL_HCURSOR,(long) 0);  //  disable class cursor
   // termout("terminal size: columns=%u, screen rows=%u", term->cols, term->rows) ;
   termout(_T("terminal size: columns=%u, screen rows=%u"),
      term_get_columns(), term_get_rows()) ;

   create_gdiplus_elements();
   //****************************************************************
   main_timer_id = SetTimer(hwnd, IDT_TIMER_MAIN, 100, (TIMERPROC) NULL) ;
}

//***********************************************************************
static LRESULT CALLBACK TermProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
   static bool main_timer_ran_once = false ;
   static uint quarter_seconds = 0 ;
   // static uint curr_redraw_counts = 0 ;
   // static uint ref_redraw_counts = 0 ;

   //***************************************************
   //  debug: log all windows messages
   //***************************************************
   if (dbg_flags & DBG_WINMSGS) {
      switch (iMsg) {
      //  list messages to be ignored
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORSTATIC:
      case WM_CTLCOLOREDIT:
      case WM_CTLCOLORDLG:
      case WM_MOUSEMOVE:
      case 295:  //  WM_CHANGEUISTATE
      case WM_NCMOUSEMOVE:
      case WM_NCMOUSELEAVE:
      case WM_NCHITTEST:
      case WM_SETCURSOR:
      case WM_ERASEBKGND:
      case WM_TIMER:
      case WM_NOTIFY:
      case WM_COMMAND:  //  prints its own msgs below
         break;
      default:
         syslog(_T("TOP [%s]\n"), lookup_winmsg_name(iMsg)) ;
         break;
      }
   }

   switch(iMsg) {
   case WM_INITDIALOG:
      do_init_dialog(hwnd) ;
      // wpOrigMainProc = (WNDPROC) SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG) MainSubclassProc); 
      return TRUE;

   //********************************************************************
   case WM_TIMER:
      switch (wParam) {
      case IDT_TIMER_MAIN:
         if (!main_timer_ran_once) {
            main_timer_ran_once = true ;
            KillTimer(hwnd, main_timer_id) ;
   
            draw_intro_screen(hwnd) ;
            // prog_init_done = true ;
            // main_timer_id = SetTimer(hwnd, IDT_TIMER_MAIN, 1000, (TIMERPROC) NULL) ;
            main_timer_id = SetTimer(hwnd, IDT_TIMER_MAIN, 244, (TIMERPROC) NULL) ;
         } else {
            if (++quarter_seconds >= 4) {
               quarter_seconds = 0 ;
               update_cursor() ;
            }
            // syslog("next_timer...redraw_in_progress=%s\n", (redraw_in_progress) ? "true" : "false") ;
            // if (redraw_in_progress) {
            //    syslog(_T("redraw in progress"));
            //    // syslog("redraw counts: %u vs %u\n", curr_redraw_counts, ref_redraw_counts) ;
            //    // if (ref_redraw_counts != 0  &&  ref_redraw_counts == curr_redraw_counts) {
            //       // syslog("redraw main screen\n") ;
            //       redraw_in_progress = false ;
            //       if (!is_intro_screen_active()) {
            //          draw_current_screen() ;
            //          show_treasures() ;
            //       }
            //    // }
            //    // ref_redraw_counts = curr_redraw_counts ;
            // }
         }
         return TRUE;

      default:
         break;
         // return DefWindowProcA(hwnd, iMsg, wParam, lParam);
      }
      break;

   case WM_NOTIFY:
      return term_notify(hwnd, lParam) ;

   //***********************************************************************************************
   //  04/16/14 - unfortunately, I cannot use WM_SIZE, nor any other message, to draw my graphics,
   //  because some other message occurs later and over-writes my work...
   //***********************************************************************************************
   // case WM_SIZE:
   //    if (wParam == SIZE_RESTORED) {
   //       syslog(_T("WM_SIZE\n")) ;
   //       redraw_in_progress = true ;
   //    } 
   //    //********************************************************************************************
   //    //  The last operations in the dialog redraw, are subclassed WM_CTLCOLORSTATIC messages.
   //    //  So, to determine when it is all done, I need to somehow recognize when these are done,
   //    //  and then update our graphics objects.
   //    //********************************************************************************************
   //    return TRUE;

   //  this occurs during program startup
   // case WM_ERASEBKGND:
   //    syslog(_T("WM_ERASEBKGND\n")) ;
   //    redraw_in_progress = true ;
   //    break;

   case WM_COMMAND:
      {  //  create local context
      DWORD cmd = HIWORD (wParam) ;
      DWORD target = LOWORD(wParam) ;

      switch (cmd) {
      case FVIRTKEY:  //  keyboard accelerators: WARNING: same code as CBN_SELCHANGE !!
         //  fall through to BM_CLICKED, which uses same targets
      case BN_CLICKED:
         if (dbg_flags & DBG_WINMSGS) {
            syslog(_T("cmd: %u, target: %u\n"), cmd, target);
         }
         switch(target) {
            
         case IDB_ABOUT:
            CmdAbout(hwnd);
            break;
            
         case IDB_DEBUG:
            toggle_debug_flag();  
            break;
            
         case IDB_DEBUG_RS:
            debug_force_runestaff();
            break;
            
         case IDB_DEBUG_LEVELS:
            dump_level_knowledge();
            break;
            
         case IDB_HELP:
            view_help_screen(hwnd);  
            break;
            
         case IDB_CLOSE:
            PostMessageA(hwnd, WM_CLOSE, 0, 0);
            break;
         } //lint !e744  switch target
         return true;
      } //lint !e744  switch cmd
      break;
      }  //lint !e438 !e10  end local context

   //********************************************************************
   //  application shutdown handlers
   //********************************************************************
   case WM_CLOSE:
      DestroyWindow(hwnd);
      break;

   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   // default:
   //    return false;
   }  //lint !e744  switch(iMsg) 

   return false;
}

//***********************************************************************
//lint -esym(1784, WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
   {
   g_hinst = hInstance;

   //Plant seed for random number generator with system time
   time_t ti ;
   time(&ti) ;
   srand2((unsigned) ti);

   load_exec_filename() ;  //  get our executable name
   //  set up initial data structs
   // read_config_data() ;
   init_gdiplus_data() ;
   init_castle_contents() ;
   init_player() ;

   HWND hwnd = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, (DLGPROC) TermProc) ;
   if (hwnd == NULL) {
      syslog(_T("CreateDialog: %s\n"), get_system_message()) ;
      return 0;
   }
   HACCEL hAccel = LoadAccelerators(g_hinst, MAKEINTRESOURCE(IDR_ACCELERATOR1));  

   MSG Msg;
   while(GetMessage(&Msg, NULL,0,0)) {
      if(!TranslateAccelerator(hwnd, hAccel, &Msg)  &&  !IsDialogMessage(hwnd, &Msg)) {
      // if(!IsDialogMessage(hwnd, &Msg)) {
          TranslateMessage(&Msg);
          DispatchMessage(&Msg);
      }
   }

   release_gdiplus_data();
   
   return (int) Msg.wParam ;
}  //lint !e715

