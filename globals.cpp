//************************************************************
//  globals.cpp
//  global variables for Wizard's Castle program
//  
//  Written by:   Daniel D. Miller
//  
//  Last Update:  03/01/01 18:52
//  
//************************************************************

#include <windows.h>
#include <tchar.h>

#include "common.h"
#include "wizard.h"

player_info_t player ;    //  player attributes

castle_room_t castle[DIMEN_COUNT][DIMEN_COUNT][DIMEN_COUNT] ; //  Castle definitions

TCHAR tempstr[128] ;

//  locations of special objects which share room with other objects
castle_room_t runestaff_room, orb_room ;
castle_room_t curse_rooms[3] ;

//**************************************************************
//  this is only referenced by wfuncs.cpp
//**************************************************************
// typedef struct object_data_s {
//    unsigned sprite_row ;
//    unsigned sprite_col ;
//    char*    desc ;
// } object_data_t ;
object_data_t object_data[LAST_OBJECT] = {
{ 25,  9, _T("unassigned room") }, // UNSEEN_ROOM    ,  
{ 21,  1, _T("normal room"    ) }, // EMPTY_ROOM     ,  
{ 21,  4, _T("entrance"       ) }, // CASTLE_ENTRANCE,  
{ 21, 11, _T("stairs up"      ) }, // STAIRS_UP      ,  
{ 21, 12, _T("stairs down"    ) }, // STAIRS_DOWN    ,  

// { OBJECT_BASE    ,   0,  6) },
{ 22, 17, _T("a pool"       ) }, // POOL           ,  
{ 14, 26, _T("a chest"      ) }, // CHEST          ,  
{ 14, 28, _T("gold pieces"  ) }, // GOLD_PIECES    ,  
{ 16,  3, _T("flares"       ) }, // FLARES         ,  
{ 22, 11, _T("a warp"       ) }, // WARP           ,  
{ 22,  2, _T("a sinkhole"   ) }, // SINKHOLE       ,  
{  0, 32, _T("a crystal orb") }, // CRYSTAL_ORB    ,  
{ 18,  0, _T("a book"       ) }, // BOOK           ,  
// { OBJECT_END     ,  26,  9) },

// { MONSTER_BASE   ,  26,  9) },
{  1, 21, _T("kobold"  ) }, // KOBOLD         ,    
{  1, 35, _T("orc"     ) }, // ORC            ,   
{  0, 22, _T("kresh"   ) }, // KRESH          ,   
{  1,  5, _T("goblin"  ) }, // GOBLIN         ,   
{  5,  5, _T("ogre"    ) }, // OGRE           ,   
{  4, 12, _T("troll"   ) }, // TROLL          ,   
{  8, 16, _T("minotaur") }, // MINOTAUR       ,   
{  9, 12, _T("ur-vile" ) }, // UR_VILE        ,   
{  1, 14, _T("gargoyle") }, // GARGOYLE       ,   
{  7, 33, _T("chimera" ) }, // CHIMERA        ,   
{  7, 17, _T("balrog"  ) }, // BALROG         ,   
{  3, 27, _T("dragon"  ) }, // DRAGON         ,   
// { MONSTER_END    ,  26,  9) },

{ 14, 34, _T("vendor") }, // VENDOR         ,  

// { TREASURE_BASE  ,  26,  9) },
{ 19, 29, _T("the Ruby Red"  ) }, // RUBY_RED       ,  
{ 19, 30, _T("the Norn Stone") }, // NORN_STONE     ,  
{ 20,  1, _T("the Pale Pearl") }, // PALE_PEARL     ,  
{ 16, 13, _T("the Opal Eye"  ) }, // OPAL_EYE       ,  
{ 19, 33, _T("the Green Gem" ) }, // GREEN_GEM      ,  
{ 19, 31, _T("the Blue Flame") }, // BLUE_FLAME     ,  
{ 20,  3, _T("the Palantir"  ) }, // PALANTIR       ,  
{ 20,  0, _T("the Silmaril"  ) }, // SILMARIL       ,  

{ 19, 25, _T("the RuneStaff" ) }, // RUNESTAFF       ,  
{  0, 33, _T("the Orb of Zot") }, // ORB_OF_ZOT       ,  
{  8, 39, _T("player") }  // PLAYER         ,  
} ;

//**************************************************************
//  misc string arrays
//**************************************************************
TCHAR *race_str[4]   = { _T("Human "),  _T("Dwarf "),  _T("Hobbit"), _T("Elf   ") } ;

