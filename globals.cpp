//************************************************************
//  globals.cpp
//  global variables for Wizard's Castle program
//  
//  Written by:   Daniel D. Miller
//************************************************************

#include <windows.h>
#include <tchar.h>

#include "common.h"
#include "wizard.h"

player_info player ;    //  player attributes

castle_room_t castle[DIMEN_COUNT][DIMEN_COUNT][DIMEN_COUNT] ; //  Castle definitions

TCHAR tempstr[128] ;

//  locations of special objects which share room with other objects
castle_room_t runestaff_room, orb_room ;
castle_room_t curse_rooms[3] ;

//**************************************************************
//  misc string arrays
//**************************************************************
TCHAR *race_str[4]   = { _T("Human "),  _T("Dwarf "),  _T("Hobbit"), _T("Elf   ") } ;

