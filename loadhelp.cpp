//************************************************************************
//  Copyright (c) 1986-2025  Daniel D Miller
//  winwiz.exe - Wizard's Castle
//  loadhelp.cpp - Load Windows help file
//************************************************************************
#include <windows.h>
#include <htmlhelp.h>
#include <tchar.h>

#include "common.h"

//*************************************************************
#ifndef PATH_MAX
#define  PATH_MAX    1024
#endif

static TCHAR chmname[PATH_MAX] = _T("");

static void find_chm_location(void)
{
   //  This requires that load_exec_filename() be previously called,
   //  typically at beginning of application.
   LRESULT result = derive_filename_from_exec(chmname, (TCHAR *) _T(".chm")) ; //lint !e1773 const
   if (result != 0) {
      syslog(_T("find_chm_loc: %s\n"), get_system_message());
   }
}

//*************************************************************
void view_help_screen(HWND hwnd)
{
   find_chm_location() ;
   
   // syslog("help=[%s]", chmname) ;
   //  MinGw gives a couple of indecipherable linker warnings about this:
   // Warning: .drectve `-defaultlib:uuid.lib ' unrecognized
   // Warning: .drectve `-defaultlib:uuid.lib ' unrecognized   
   //  But ignoring them doesn't seem to hurt anything...
   HtmlHelp(hwnd, chmname, HH_DISPLAY_TOPIC, 0L);
   return ;
}

