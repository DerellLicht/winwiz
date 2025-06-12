//************************************************************
//  globals.cpp
//  global variables for Wizard's Castle program
//  
//  Written by:   Derell Licht
//************************************************************

#include <windows.h>
#include <tchar.h>

#include "common.h"
#include "wizard.h"

player_info player ;    //  player attributes

//lint -esym(729, castle)  Symbol not explicitly initialized
//lint -esym(552, castle)  Symbol not accessed
castle_room castle[DIMEN_COUNT][DIMEN_COUNT][DIMEN_COUNT] ; //  Castle definitions

TCHAR tempstr[128] ;

//  locations of special objects which share room with other objects
castle_room runestaff_room, orb_room ;
castle_room curse_rooms[3] ;

//**************************************************************
//  misc string arrays
//**************************************************************
TCHAR *race_str[4]   = { _T("Human "),  _T("Dwarf "),  _T("Hobbit"), _T("Elf   ") } ;

