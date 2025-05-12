//****************************************************************************
//  Copyright (c) 1985-2025  Daniel D Miller
//  winwiz.exe - Win32 version of Wizard's Castle
//  wfuncs.cpp - handle data update and rendering functions
//
//  Written by:  Dan Miller
//****************************************************************************
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  //  rand() 
#include <tchar.h>
//  string plus vector classes add 78KB to program
#include <string>
#include <vector>

#include "resource.h"
#include "common.h"
#include "commonw.h"
#include "wizard.h"
#include "keywin32.h"
#include "terminal.h" 
#ifdef UNICODE
#include "gdi_plus.h"
#else
#include "lode_png.h"
#endif

//@@@  why do I need this here?? (as min() )
//@@@  It *should* be defined in windef.h
#define dmin(a, b)  (((a) < (b)) ? (a) : (b)) 

//lint -esym(714, clear_room)
//lint -esym(759, clear_room)
//lint -esym(765, clear_room)

//lint -esym(715, hdc, hwnd)

enum {
TILE_COMBAT=0,
TILE_VICTORY,
TILE_DEATH
} ;

//***********************************************************************
//  sprite handler constants
//***********************************************************************
#define  SPRITE_WIDTH      32
#define  SPRITE_HEIGHT     32

#define  IMAGE_WIDTH       359
#define  IMAGE_HEIGHT      362

#ifdef UNICODE
//  tiles32.png: 1280x960, 40x26 images, 1017 total
// static gdi_plus pngSprites(_T("tiles32.png"), 40, 26) ;
static gdi_plus *pngSprites = NULL ;
//  images.png:  1077x362, 3x1 images, 3 total
// static gdi_plus pngTiles  (_T("images.png"), 3, 1) ;
static gdi_plus *pngTiles = NULL;
#else
static LodePng pngSprites(_T("tiles32.png"), SPRITE_HEIGHT, SPRITE_WIDTH) ;
static LodePng pngTiles  (_T("images.png"),  IMAGE_WIDTH,   IMAGE_HEIGHT) ;
#endif

//*************************************************************
#define  X_OFFSET    16
#define  X_GAP       10

#define  Y_OFFSET    16
#define  Y_GAP       10

//*************************************************************
static HWND hwndTreasures[10] ;
static HWND hwndMapArea  = NULL ;
static HWND status_windows[8] ;

static bool level_known[DIMEN_COUNT] = { false, false, false, false, false, false, false, false };
// static bool location_forgotten = false ;

typedef enum map_image_e {
MI_UNDEFINED=0,
MI_MAP,
MI_COMBAT,
MI_VICTORY,
MI_DEATH
} map_image_t ;
static map_image_t map_image = MI_UNDEFINED ;

//***********************************************************************
//  string constants
//***********************************************************************
//  string plus list classes add 75KB to program
//  #include <list>  //  does not offer random access...
// Access third element
// cout << *next(l.begin(), 2);

// static wchar_t *names[10] = {
static std::vector<std::wstring> names {
   L"THE REALM OF BYDL", L"GRIMMERDHORE",   L"DRAGON'S EGG",    L"RYJECK THOME",
   L"SABTO'S DEMISE",    L"LYDEN VELDT",    L"DERELICT'S DOOM", 
   L"MAXWELL'S FIELD",   L"SEER'S SOJOURN", L"TREACHER'S GORGE"
};

// static wchar_t *weapon_str[5] = { L"Hands ",  L"Dagger",  L"Mace  ", L"Sword ", L"A Book" } ;
// static wchar_t *armour_str[4] = { L"Prayers", L"Leather", L"Chainmail", _T("Plate" } ;
// static wchar_t *curse_str[3]  = { L"CURSE OF LETHARGY"), L"CURSE OF THE LEECH", L"CURSE OF AMNESIA" } ;
static std::vector<std::wstring> weapon_str { L"Hands ",  L"Dagger",  L"Mace  ", L"Sword ", L"A Book" } ;
static std::vector<std::wstring> armour_str { L"Prayers", L"Leather", L"Chainmail", _T("Plate" } ;
static std::vector<std::wstring> curse_str  { L"CURSE OF LETHARGY"), L"CURSE OF THE LEECH", L"CURSE OF AMNESIA" } ;

//***********************************************************************
static void react_to_room(HWND hwndUnused);
static void show_flares(void);
static void show_str(void);
static void mark_room_as_known(unsigned x, unsigned y, unsigned level);
static void draw_char_cursor(HDC hdc, unsigned on_or_off);

/************************************************************************/
static bool is_location_forgotten(void)
{
   return !level_known[player.level] ;
}

/************************************************************************/
void create_gdiplus_elements(void)
{
   // tiles32.png: image: 1280x960, tiles: 40x27, sprites: 32x32
   pngSprites = new gdi_plus(_T("tiles32.png"), 40, 27, SPRITE_WIDTH, SPRITE_HEIGHT) ;
   
   // images.png: image: 1077x362, tiles: 3x1, sprites: 359x362
   pngTiles = new gdi_plus(_T("images.png"), 3, 1) ;
}

/************************************************************************/
void dump_level_knowledge(void)
{
   _stprintf(tempstr, _T("Levels known: 0%c 1%c 2%c 3%c 4%c 5%c 6%c 7%c, level=%u"),
      (level_known[0]) ? 'T' : 'F', (level_known[1]) ? 'T' : 'F', (level_known[2]) ? 'T' : 'F',
      (level_known[3]) ? 'T' : 'F', (level_known[4]) ? 'T' : 'F', (level_known[5]) ? 'T' : 'F',
      (level_known[6]) ? 'T' : 'F', (level_known[7]) ? 'T' : 'F', player.level) ;
   termout(tempstr);
   
   _stprintf(tempstr, _T("kbd state: %u"), get_keymap_state());
   termout(tempstr);
}

//*************************************************************
static unsigned get_room_contents(unsigned x, unsigned y, unsigned level)
{
   return castle[x][y][level].contents ;
}

unsigned get_room_contents(void)
{
   return castle[player.x][player.y][player.level].contents ;
}

//*************************************************************
TCHAR *get_room_contents_str(void)
{  
   return object_data[get_room_contents()].desc ;
}

//*************************************************************
static TCHAR *get_object_in_room(int x, int y, int level)
{  
   return object_data[get_room_contents(x, y, level)].desc ;
}

//*************************************************************
TCHAR *get_object_name(int index)
{
   return object_data[index].desc ;
}         

/************************************************************************/
//  used only for Runestaff and OrbOfZot fields
/************************************************************************/
static void clear_treasure_area(HWND hwnd, COLORREF Color)
{
   RECT   rect ;
   SetRect (&rect, 0, 0, SPRITE_WIDTH, SPRITE_HEIGHT) ;
   HDC hdc = GetDC(hwnd) ;
   HBRUSH hBrush = CreateSolidBrush (Color) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
   ReleaseDC(hwnd, hdc) ;
}

//***********************************************************************
// static void draw_sprite_treasure(HWND hwnd, unsigned scol, unsigned srow, unsigned xidest, unsigned yidest)
static void draw_sprite_treasure(unsigned scol, unsigned srow, unsigned idx)
{
   HWND hwnd = hwndTreasures[idx] ;
   HDC hdc = GetDC(hwnd) ;
   pngSprites->render_bitmap(hdc, 0, 0, scol, srow) ;
   ReleaseDC(hwnd, hdc) ;
}

//***********************************************************************
// static char *sprite_img_name = "tiles32.jpg";
static void draw_sprite(HDC hdc, unsigned scol, unsigned srow, unsigned xidest, unsigned yidest)
{
   if (xidest >= 8  ||  yidest >= 8) {
      syslog(_T("draw_sprite: invalid pos: col=%u, row=%u\n"), xidest, yidest) ;
      return ;
   }
   unsigned xdest = X_OFFSET + (xidest * (SPRITE_WIDTH  + X_GAP)) ;  //  draw_sprite()
   unsigned ydest = Y_OFFSET + (yidest * (SPRITE_HEIGHT + Y_GAP)) ;  //  draw_sprite()
   pngSprites->render_bitmap(hdc, xdest, ydest, scol, srow) ;
}

//***********************************************************************
static void draw_empty_room(HDC hdc, unsigned xidest, unsigned yidest)
{
   if (xidest >= 8  ||  yidest >= 8) {
      syslog(_T("draw_empty_room: invalid pos: col=%u, row=%u\n"), xidest, yidest) ;
      return ;
   }
   unsigned xdest = X_OFFSET + (xidest * (SPRITE_WIDTH  + X_GAP)) ;  //  draw_sprite()
   unsigned ydest = Y_OFFSET + (yidest * (SPRITE_HEIGHT + Y_GAP)) ;  //  draw_sprite()
   ul2uc_t uconv ;
   uconv.uc[2] = 107 ;  //  blue
   uconv.uc[1] = 107 ;  //  green
   uconv.uc[0] = 66 ;   //  red
   pngSprites->DrawBoxFilled(hdc, xdest, ydest, SPRITE_WIDTH, SPRITE_HEIGHT, uconv.ul);
}

//***********************************************************************
static void draw_room_contents(HDC hdc, unsigned col, unsigned row)
{
   unsigned content ;

   if (castle[col][row][player.level].is_known)
      content = get_room_contents(col, row, player.level) ;
   else
      content = UNSEEN_ROOM ;

   //  if content == EMPTY_ROOM (1), we should do a flood-fill of the space, 
   //  instead of using that icon, since that icon has a black edge around the frame,
   //  which isn't really what we want...
   if (content == EMPTY_ROOM) {
      draw_empty_room(hdc, col, row) ;
   }
   else {
      draw_sprite(hdc, object_data[content].sprite_col, 
                       object_data[content].sprite_row, col, row) ;
   }
}

//***********************************************************************
static void draw_all_room_sprites(void)
{
   HDC hdc = GetDC(hwndMapArea) ;
   for (unsigned row=0; row<8; row++) 
      for (unsigned col=0; col<8; col++) 
         draw_room_contents(hdc, col, row) ;
   ReleaseDC(hwndMapArea, hdc) ;
}

//***********************************************************************
static void draw_all_room_sprites(HDC hdc)
{
   for (unsigned row=0; row<8; row++) 
      for (unsigned col=0; col<8; col++) 
         draw_room_contents(hdc, col, row) ;
}

/************************************************************************/
//  screen-positioning constants
// #define  HORZ_DIVIDER   360
// #define  VERT_DIVIDER   360

static void clear_map_area(HDC hdc, COLORREF Color)
{
   RECT   rect ;
   // SetRect (&rect, 0, 0, HORZ_DIVIDER, VERT_DIVIDER-1) ;
   GetClientRect(hwndMapArea, &rect) ;
   // rect.left   += 1 ;
   // rect.top    += 1 ;
   rect.right  -= 1 ;
   // rect.bottom -= 1 ;
   HBRUSH hBrush = CreateSolidBrush (Color) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

//*************************************************************
void clear_room(void)
{
   castle[player.x][player.y][player.level].contents = EMPTY_ROOM ;
}

//*************************************************************
static void update_position(void)
{
   // if (player.is_blind  ||  is_location_forgotten()) 
   if (player.is_blind) 
      _stprintf (tempstr, _T(" X=?, Y=?"));
   else
      _stprintf (tempstr, _T(" X=%u, Y=%u"), player.x, player.y);
   status_message(1, tempstr);
   
   if (player.is_blind  ||  is_location_forgotten()) 
      _stprintf (tempstr, _T(" level=?"));
   else
      _stprintf (tempstr, _T(" level=%u"), player.level);
   status_message(2, tempstr);
}

//***********************************************************************
//  called by *many* functions
//***********************************************************************
void draw_main_screen(HDC hdcUnused)
{
   if (cxClient == 0 || cyClient == 0)
        return ;
   // syslog(_T("draw_main_screen\n"));
   map_image = MI_MAP ;
   // redraw_game_screen() ;
   HDC hdc = GetDC(hwndMapArea) ;
   clear_map_area(hdc, WIN_BLACK) ;
   draw_all_room_sprites(hdc);   //  using hwndMapArea
   show_player() ;
   update_position() ;
   ReleaseDC(hwndMapArea, hdc) ;
   set_term_attr_default();
   update_status() ;
}

//****************************************************************************
static void draw_current_screen(void)
{
   HDC hdc ;
   // syslog(_T("draw_current_screen\n"));
   switch (map_image) {
   case MI_COMBAT:
      hdc = GetDC(hwndMapArea) ;
      pngTiles->render_bitmap(hdc, 0, 0, TILE_COMBAT, 0) ;
      ReleaseDC(hwndMapArea, hdc) ;
      break;
   case MI_VICTORY:
      hdc = GetDC(hwndMapArea) ;
      pngTiles->render_bitmap(hdc, 0, 0, TILE_VICTORY, 0) ;
      ReleaseDC(hwndMapArea, hdc) ;
      break;
   case MI_DEATH:  
      hdc = GetDC(hwndMapArea) ;
      pngTiles->render_bitmap(hdc, 0, 0, TILE_DEATH, 0) ;
      ReleaseDC(hwndMapArea, hdc) ;
      break;
      
   case MI_UNDEFINED:
   case MI_MAP:
   default:   
      draw_main_screen(NULL) ;   // draw_current_screen()
      break;
   }
}

//****************************************************************************
static void restore_room(HDC hdc)   //  derived from hwndMapArea
{
   draw_char_cursor(hdc, OFF) ;
   draw_room_contents(hdc, player.x, player.y);
}

//*************************************************************
void show_player(void)
{
   if (!player.is_blind) {
      HDC hdc = GetDC(hwndMapArea) ;
      draw_sprite(hdc, object_data[PLAYER].sprite_col,
                       object_data[PLAYER].sprite_row, player.x, player.y) ;
      draw_char_cursor(hdc, ON) ;
      ReleaseDC(hwndMapArea, hdc) ;
   }
   update_position() ;
}

/************************************************************************/
// ;* Description: Draw line from (x0,y0) to (x1,y1) using color 'Color'   *
/************************************************************************/
static void Box(HDC hdc, int x0, int y0, int x1, int y1, unsigned Color)
{
   static HPEN hPen ;

   hPen = CreatePen(PS_SOLID, 1, Color) ;
   SelectObject(hdc, hPen) ;

   MoveToEx(hdc, x0, y0, NULL) ;
   LineTo  (hdc, x1, y0) ;
   LineTo  (hdc, x1, y1) ;
   LineTo  (hdc, x0, y1) ;
   LineTo  (hdc, x0, y0) ;

   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;
}

//***********************************************************************
static COLORREF cursor_frames[3] = {
   WIN_GREEN, WIN_BMAGENTA, WIN_BGREEN
} ;
static uint cursor_idx = 0 ;

static void draw_char_cursor(HDC hdc, unsigned on_or_off)
{
   //  calculate actual bounds
   unsigned x0 = X_OFFSET + (player.x * (SPRITE_WIDTH  + X_GAP)) ;   //  cursor computation
   unsigned y0 = Y_OFFSET + (player.y * (SPRITE_HEIGHT + Y_GAP)) ;   //  cursor computation
   unsigned x1 = x0 + SPRITE_WIDTH ;
   unsigned y1 = y0 + SPRITE_HEIGHT ;
   unsigned Color = WIN_BLACK ;

   if (on_or_off == ON)
      Color = cursor_frames[cursor_idx % 3] ;
   x0-- ;
   y0-- ;
   Box(hdc, x0, y0, x1, y1, Color) ;
   if (on_or_off == ON)
      Color = cursor_frames[(cursor_idx+1) % 3] ;
   x0-- ;
   y0-- ;
   x1++ ;
   y1++ ;
   Box(hdc, x0, y0, x1, y1, Color) ;
   if (on_or_off == ON)
      Color = cursor_frames[(cursor_idx+2) % 3] ;
   x0-- ;
   y0-- ;
   x1++ ;
   y1++ ;
   Box(hdc, x0, y0, x1, y1, Color) ;
}

//*************************************************************
void update_cursor(void)
{
   HDC hdc ;
   if (is_default_keymap()  &&  !player.is_blind) {
      if (++cursor_idx >= 3)
         cursor_idx = 0 ;

      switch (map_image) {
      case MI_MAP:
         hdc = GetDC(hwndMapArea) ;
         draw_char_cursor(hdc, true) ;
         ReleaseDC(hwndMapArea, hdc) ;
         break;
         
      case MI_UNDEFINED:
      case MI_COMBAT:
      case MI_VICTORY:
      case MI_DEATH:  
      default:   
         // syslog(_T("update_cursor()\n"));
         // draw_main_screen(NULL) ;   //  update_cursor()
         break;
      }
   }
}

//****************************************************************************
static void mark_room_as_known(unsigned x, unsigned y, unsigned level)
{
   castle[x][y][level].is_known = 1 ;
}

static void hide_room(unsigned x, unsigned y, unsigned level)
{
   castle[x][y][level].is_known = 0 ;
}

//*************************************************************
void render_combat_bitmap(void)
{
   HDC hdc = GetDC(hwndMapArea) ;
   map_image = MI_COMBAT ;
   pngTiles->render_bitmap(hdc, 0, 0, TILE_COMBAT, 0) ;
   ReleaseDC(hwndMapArea, hdc) ;
}

//************************************************************************************
//  This function is one of the optional operations that may be invoked when
//  a chest is opened.  The original design attempted to add up the existing
//  attribute values and randomly re-distribute them, but that is difficult to
//  do in a truly balanced fashion.
//  
//  Instead, this modified routine (from 09/08/23) randomly generates new attributes
//  from the theoretical maximum values.  
//  It may generate a total amount which is more or less than the original total, 
//  but it has advantage of being a truly random distribution.
//************************************************************************************
static void ScrambleAttr(void)
{
   unsigned j=0, k=0, l=0 ;
   
#define  USE_BASIC_SYSTEM   

#ifdef  USE_BASIC_SYSTEM
#define  MAX_SINGLE_ATTR   18

   j = 1 + random_int(MAX_SINGLE_ATTR);
   k = 1 + random_int(MAX_SINGLE_ATTR);
   l = 1 + random_int(MAX_SINGLE_ATTR);
#else
   //  This method favours STR over DEX, and DEX over INT.
   unsigned attr_temp = player.str + player.dex + player.iq ;

   j = (1 + random(min(attr_temp, 18))) ;
   attr_temp -= j ;
   if (attr_temp == 0)  goto skipping ;
   k = (1 + random(min(attr_temp, 18))) ;
   attr_temp -= k ;
   if (attr_temp == 0)  goto skipping ;
   l = (1 + random(min(attr_temp, 18))) ;
   attr_temp -= l ;
skipping:
   // printf("init str=%u, dex=%u, int=%u, j=%u, k=%u, l=%u, temp=%u\n", 
   //    str, dex, iq, j, k, l, attr_temp) ;

   //  distribute remaining points evenly across all stats
   while (attr_temp != 0) {
      unsigned touched = 0 ;
      if (j < 18) {
         j++ ;
         touched++ ;
         if (--attr_temp == 0)
            continue;
      }
      if (k < 18) {
         k++ ;
         touched++ ;
         if (--attr_temp == 0)
            continue;
      }
      if (l < 18) {
         l++ ;
         touched++ ;
         if (--attr_temp == 0)
            continue;
      }
      if (!touched) {
         _stprintf(tempstr, "no touch: str=%u, dex=%u, int=%u\n", j, k, l) ;
         status_message(tempstr);
         break;
      }
   }
#endif   

   //  attribute selection here should also be random...
   player.str = j ;
   player.dex = k ;
   player.iq  = l ;
   adjust_hit_points() ;
   
   update_status() ;
}

//*************************************************************
void win_game(HWND hwnd)
{
   push_keymap(KEYMAP_WAIT_END) ;
   HDC hdc = GetDC(hwndMapArea) ;
   clear_map_area(hdc, WIN_BLACK) ;
   map_image = MI_VICTORY ;
   pngTiles->render_bitmap(hdc, 0, 0, TILE_VICTORY, 0) ;
   ReleaseDC(hwndMapArea, hdc) ;
   term_clear_message_area();
   put_message(_T("You find yourself standing beside a sweet, cool")) ;
   put_message(_T("stream in the shade of a glowing, verdant forest...")) ;
   put_message(_T(" ")) ;
   _stprintf(tempstr, _T("You emerge Victorious from %s !!!"), names[player.castle_nbr].c_str()) ;
   put_message(tempstr) ;
}

//*************************************************************
static bool starts_with_vowel(TCHAR *monster)
{
   switch (*monster) {
   case 'a':
   case 'e':
   case 'i':
   case 'o':
   case 'u':
      return true;

   default:
      return false;
   }
}

//*************************************************************
TCHAR *get_monster_prefix(TCHAR *monster_name)
{
   static TCHAR *an_str = _T("an") ;
   static TCHAR *a_str = _T("a") ;
   return starts_with_vowel(monster_name) ? an_str : a_str ;
}
//*************************************************************
static void display_atmosphere(void)
{
   if (player.has_orb_of_Zot) {
      put_color_msg(TERM_DEATH, _T("In a burst of intense agony, you feel your very soul ripped from")) ;
      put_color_msg(TERM_DEATH, _T("your body, and sucked into the Orb of Zot, which drops to the ground")) ;
      put_color_msg(TERM_DEATH, _T("with a soft 'tink', and sinks softly below the surface.")) ;
      put_color_msg(TERM_DEATH, _T("Oddly, you find you are not alone within the Orb... in fact, your existance")) ;
      put_color_msg(TERM_DEATH, _T("promises to become quite interesting now - but that's another story.")) ;
   } else
   if (player.has_runestaff) {
      put_color_msg(TERM_DEATH, _T("With your last fading breath, you gasp a curse upon %s!!"),
         names[player.castle_nbr].c_str()) ;
      put_color_msg(TERM_DEATH, _T("The power of your curse causes the Runestaff to shatter with")) ;
      put_color_msg(TERM_DEATH, _T("a catastrophic blast, collapsing the castle upon itself...")) ;
      put_color_msg(TERM_DEATH, _T("Surely the Wizard of Zot would be gratified.")) ;
   } else 
   {
      put_color_msg(TERM_DEATH, _T("You died while exploring %s"), names[player.castle_nbr].c_str()) ;
      // char *sptr = get_object_in_room(player.x, player.y, player.level) ;
      TCHAR *sptr = get_object_in_room(runestaff_room.x, runestaff_room.y, runestaff_room.level) ;
      put_color_msg(TERM_DEATH, _T("%s %s declares itself Master of the Castle!!"),
         get_monster_prefix(sptr), sptr) ;
   }
}

//*************************************************************
void player_dies(HWND hwnd)
{
   push_keymap(KEYMAP_WAIT_END) ;
   update_status() ;
   display_atmosphere() ;
   map_image = MI_DEATH ;
   draw_current_screen();
}

//*************************************************************
static TCHAR *zot_chest_end_msg[] = {
_T("The wizard continues speaking as you respond, but his voice"),
_T("grows ever fainter, until it fades from hearing..."),
_T(" "),
_T("You grow colder and colder as your vision fades......."),
_T(" "),
0 } ;
   
static void wrong_castle_death(HWND hwnd)
{
   term_clear_message_area() ;
   for (unsigned j=0; zot_chest_end_msg[j] != 0; j++) {
      // put_color_msg(TERM_DEATH, zot_chest_end_msg[j]) ;
      put_color_msg(TERM_DEATH, zot_chest_end_msg[j]) ;
   }
   infoout(_T("You died while exploring %s"), names[player.castle_nbr].c_str()) ;
   map_image = MI_DEATH ;
   draw_current_screen();
}

//*************************************************************
static TCHAR *first_zot_chest_msg[] = {
_T("The room fills with smoke that has an odd odor."),
_T("The smoke seems to bite into your skin."),
_T("Anxiety thrills through your body...."),
_T(" "),
_T("You are blinded for a moment, but then"),
_T("the image of a man appears before you."),
_T("He is old and wizened, but seems strong despite his age."),
0 } ;

static TCHAR *second_zot_chest_msg[] = {
_T("This smoke is strong with the power of ZOT......"),
_T("You are too weak to resist its effects, and you "),
_T("are now floating far, far away into the smoke..."),
_T("                                                   "),
_T("I may be able to save you if you can tell me"),
_T("WHICH CASTLE you are from ....."),
_T("Please speak quickly before you are out of reach!!"),
0 } ;

//*************************************************************
// #define  USE_MESSAGE_WINDOW   1

#define  MAX_CASTLE_NAME   20
static TCHAR castle_name[MAX_CASTLE_NAME+1] ;
static unsigned cn_len ;
#ifdef  USE_MESSAGE_WINDOW
static unsigned zot_name_row = 0 ;
#endif

//*************************************************************
static int process_zot_keystroke (HWND hwnd, unsigned inchr)
{
   // _stprintf(tempstr, "%X ", inchr) ;
   // OutputDebugString(tempstr) ;
   if (inchr & 0xFF00) {
      _stprintf (tempstr, _T("PRESS=0x%04X"), inchr);
      status_message(tempstr);
   } 
   //  process normal keys
   else if (inchr == kENTER) {
      // return (cn_len == 0) ? -1 : (int) cn_len ;
      return (int) cn_len ;
   }
   else if (inchr == kBSPACE) {
      if (cn_len > 0) {
#ifdef  USE_MESSAGE_WINDOW
         //  over-write existing name with black-on-black
         dprints_centered_x(hwnd, zot_name_row, 0, 0, castle_name) ;

         //  modify name and redraw it
         cn_len-- ;
         castle_name[cn_len] = 0 ;  //  make sure string stays NULL-term
         dprints_centered_x(hwnd, zot_name_row, WIN_BGREEN, 0, castle_name) ;
#else
         //  modify name and redraw it
         cn_len-- ;
         castle_name[cn_len] = 0 ;  //  make sure string stays NULL-term
         wterm_replace(castle_name) ;
#endif
      }
   }   
   else {
      if (cn_len < MAX_CASTLE_NAME) {
         // _stprintf(tempstr, "%u ", inchr) ;
         // OutputDebugString(tempstr) ;
         //  for some reason, single-quotes are coming in as 0xDE ...
         if ((u8) inchr == 0xDE) {
            inchr = 0x27 ;
         }
         castle_name[cn_len] = (char) inchr ;
         cn_len++ ;
         castle_name[cn_len] = 0 ;  //  make sure string stays NULL-term
#ifdef  USE_MESSAGE_WINDOW
         dprints_centered_x(hwnd, zot_name_row, WIN_BGREEN, 0, castle_name) ;
#else
         wterm_replace(castle_name) ;
#endif
      }
   }
   return 0;   //  indicates more data follows
}

//*********************************************************
int manage_zot_input(HWND hwnd, unsigned inchr)
{
   int result = process_zot_keystroke (hwnd, inchr);
   if (result < 0) {
      //  treat -1 as automatic fail
      // OutputDebugString("automatic fail??") ;
      wrong_castle_death(hwnd) ;
      result = -1 ;
   }
   else if (result > 0) {
      //  compare entered data vs castle name
      // TCHAR *sptr = names[player.castle_nbr] ;
      // u_int slen = _tcslen(names[sptr);
      const wchar_t *sptr = names[player.castle_nbr].c_str() ;
      u_int slen = names[player.castle_nbr].length() ;
      if (_tcsnicmp(castle_name, sptr, slen) == 0) {
         result = 1 ;
         infoout(_T("Whew!!!")) ;
         // hide_status_area() ;
         // clear_dialog_area(hdcMain, GetSysColor(COLOR_3DFACE));
         // enable_player_info(false) ;
      } else {
         // clear_dialog_area(hdcMain, GetSysColor(COLOR_3DFACE));
         wrong_castle_death(hwnd) ;
         result = -1 ;
      }
   }
   return result ;
}

//*********************************************************
static void draw_zot_window(HWND hwnd)
{
   unsigned j ;
   
   term_clear_message_area() ;
   // clear_dialog_area(hdcMain, WIN_BLACK);
   cn_len = 0 ;
   castle_name[cn_len] = 0 ;  //  make sure string stays NULL-term
   // void dprints_centered_x(HDC hdc, LONG y, unsigned attr, char *str)
#ifdef  USE_MESSAGE_WINDOW
   unsigned theight ;
   static unsigned y = 0 ;
   HDC hdc = hdcMain ;  //  ifdef USE_MESSAGE_WINDOW
   myTerminal->set_terminal_font(_T("Bodacious-Normal"), 110, EZ_ATTR_NORMAL) ;
   set_text_font(_T("Bodacious-Normal"), 110) ;
   theight = textheight(hdc, 0) ;
   y = 2 * theight ;
   for (j=0; first_zot_chest_msg[j] != 0; j++) {
      dprints_centered_x(hdc, y, WIN_BRED, first_zot_chest_msg[j]) ;
      y += theight ;
   }
   y += theight ;
   dprints_centered_x(hdc, y, WIN_BMAGENTA, _T(" He speaks: ")) ;
   y += theight ;
   y += theight ;
   for (j=0; second_zot_chest_msg[j] != 0; j++) {
      dprints_centered_x(hdc, y, WIN_BRED, second_zot_chest_msg[j]) ;
      y += theight ;
   }
   y += theight ;
   zot_name_row = y ;
#else
   for (j=0; first_zot_chest_msg[j] != 0; j++) {
      put_color_msg(TERM_PLAYER_HIT, first_zot_chest_msg[j]) ;
   }
   put_message(WIN_BMAGENTA, WIN_BLACK, _T(" He speaks: ")) ;
   for (j=0; second_zot_chest_msg[j] != 0; j++) {
      put_color_msg(TERM_PLAYER_HIT, second_zot_chest_msg[j]) ;
   }
#endif
}

//*********************************************************
static void CheckCurses(HDC hdc) //  derived from hwndMapArea
{
   unsigned x, y, level, itemp ;

   player.turns++ ;
   
   /*       CURSE OF LETHARGY  */
   //  treasure[0] (Ruby Red) cures lethargy
   //  not currently used
   if (player.curse_flags & CR_LETHARGY) {
      if (player.treasures[TR_RUBY_RED]) {
         player.curse_flags &= ~CR_LETHARGY ;
         put_message(_T("The Ruby Red cures the Curse of Lethargy...")) ;
      } else {
         
      }
   }

   /*       CURSE OF THE LEECH */
   //  treasure[2] (Pale Pearl) cures leech
   if (player.curse_flags & CR_LEECH) {
      if (player.treasures[TR_PALE_PEARL]) {
         player.curse_flags &= ~CR_LEECH ;
         put_message(_T("The Pale Pearl cures the Curse of the Leech...")) ;
      } else {
         itemp = random(5);
         if (itemp >= player.gold) {
            player.gold = 0 ;
         } else {
            player.gold -= itemp ;
         }
         show_gold() ;
      }
   }

   /*       CURSE OF FORGETFULNESS */
   //  treasure[4] (Green Gem) cures forgetfulness
   if (player.curse_flags & CR_FORGET) {
      if (player.treasures[TR_GREEN_GEM]) {
         player.curse_flags &= ~CR_FORGET ;
         put_message(_T("The Green Gem cures the Curse of Forgetfulness...")) ;
      } else {
         x = random(DIMEN_COUNT); y = random(DIMEN_COUNT); level = random(DIMEN_COUNT);
         hide_room(x, y, level) ;
         if (level == player.level) {
            draw_room_contents(hdc, x, y) ;
         }
      }
   }

   //  did we step on a new curse??
   if (curse_rooms[CURSE_OF_LETHARGY].is_known == 0  &&
       curse_rooms[CURSE_OF_LETHARGY].x == player.x  &&
       curse_rooms[CURSE_OF_LETHARGY].y == player.y  &&
       curse_rooms[CURSE_OF_LETHARGY].level == player.level) {
         
      curse_rooms[CURSE_OF_LETHARGY].is_known = 1 ;
      if (!(player.treasures[TR_RUBY_RED])) {
         player.curse_flags |= CR_LETHARGY ;
         put_message(_T("***  You have been afflicted with the Curse of Lethargy !!")) ;
      }
   }
   if (curse_rooms[CURSE_OF_LEECH].is_known == 0  &&
       curse_rooms[CURSE_OF_LEECH].x == player.x  &&
       curse_rooms[CURSE_OF_LEECH].y == player.y  &&
       curse_rooms[CURSE_OF_LEECH].level == player.level) {
         
      curse_rooms[CURSE_OF_LEECH].is_known = 1 ;
      if (!(player.treasures[TR_PALE_PEARL])) {
         player.curse_flags |= CR_LEECH ;
         put_message(_T("***  You have been afflicted with the Curse of the Leech !!")) ;
      }
   }
   if (curse_rooms[CURSE_OF_FORGET].is_known == 0  &&
       curse_rooms[CURSE_OF_FORGET].x == player.x  &&
       curse_rooms[CURSE_OF_FORGET].y == player.y  &&
       curse_rooms[CURSE_OF_FORGET].level == player.level) {
         
      curse_rooms[CURSE_OF_FORGET].is_known = 1 ;
      if (!(player.treasures[TR_GREEN_GEM])) {
         player.curse_flags |= CR_FORGET ;
         put_message(_T("***  You have been afflicted with the Curse of Amnesia !!")) ;
      }
   }
}  

//*********************************************************
static TCHAR *comment_str[9] = {
_T("You stepped on a toad..."),
_T("You hear a scream!!"),
_T("You hear someone breathing!!"),
_T("You hear a Limnphedon!"),
_T("You hear thunder in the distance."),
_T("You feel bugs on your face !"),
_T("You smell a fried monster"),
_T("You see something glowing in the distance !"),
_T("You hear faint rustling noises..."   )
} ;

static void Comments(void)
{
   if (random(5) == 1) {
      put_color_msg(TERM_ATMOSPHERE, comment_str[random(9)]) ;
   }
}

//*************************************************************
static bool is_monster_index(uint idx)
{
   if (idx >= MONSTER_BASE  &&  idx <= MONSTER_END) 
      return true;
   return false;
}

//*************************************************************
static void update_room(void)
{
   if (player.is_blind) 
      return ;
   
   HDC hdc = GetDC(hwndMapArea) ;
   mark_room_as_known(player.x, player.y, player.level) ;
   show_player() ;
   if (player.has_lamp) {
      if (player.x > 0) {
         mark_room_as_known(player.x-1, player.y, player.level) ;
         draw_room_contents(hdc, player.x-1, player.y) ;
      }
      if (player.x < 7) {
         mark_room_as_known(player.x+1, player.y, player.level) ;
         draw_room_contents(hdc, player.x+1, player.y) ;
      }
      if (player.y > 0) {
         mark_room_as_known(player.x, player.y-1, player.level) ;
         draw_room_contents(hdc, player.x, player.y-1) ;
      }
      if (player.y < 7) {
         mark_room_as_known(player.x, player.y+1, player.level) ;
         draw_room_contents(hdc, player.x, player.y+1) ;
      }
   }

   //  do not repeatedly display the "here you find normal room" message
   uint idx = get_room_contents() ;
   if (idx != EMPTY_ROOM) {
      // if (idx >= MONSTER_BASE  &&  idx <= MONSTER_END) {
      if (is_monster_index(idx)) {
         TCHAR *sptr = get_object_in_room(player.x, player.y, player.level) ;
         _stprintf(tempstr, _T("Here you find %s %s."), 
            get_monster_prefix(sptr), sptr) ;
      } else {
         _stprintf(tempstr, _T("Here you find %s."), 
            get_object_in_room(player.x, player.y, player.level)) ;
      }
      put_message(tempstr) ;
   }
   ReleaseDC(hwndMapArea, hdc) ;
}

//****************************************************************************
int move_west(HWND hwnd)
{
   if (player.x == 0)
      return 1 ;
   HDC hdc = GetDC(hwndMapArea) ;
   restore_room(hdc) ;
   player.x-- ;
   update_room() ;
   react_to_room(NULL) ;
   ReleaseDC(hwndMapArea, hdc) ;
   Comments() ;
   return 0;
}

//****************************************************************************
int move_east(HWND hwnd)
{
   if (player.x == 7)
      return 1 ;
   HDC hdc = GetDC(hwndMapArea) ;
   restore_room(hdc) ;
   player.x++ ;
   update_room() ;
   react_to_room(NULL) ;
   ReleaseDC(hwndMapArea, hdc) ;
   Comments() ;
   return 0;
}

//****************************************************************************
int move_north(HWND hwnd)
{
   if (player.y == 0)
      return 1 ;
   HDC hdc = GetDC(hwndMapArea) ;
   restore_room(hdc) ;
   player.y-- ;
   update_room() ;
   react_to_room(NULL) ;
   ReleaseDC(hwndMapArea, hdc) ;
   Comments() ;
   return 0;
}

//****************************************************************************
int move_south(HWND hwnd)
{
   if (player.y == 7)
      return 1 ;
   HDC hdc = GetDC(hwndMapArea) ;
   restore_room(hdc) ;
   player.y++ ;
   update_room() ;
   react_to_room(NULL) ;
   ReleaseDC(hwndMapArea, hdc) ;
   Comments() ;
   return 0;
}

//****************************************************************************
int move_down(HWND hwnd)
{
   int room_chr = get_room_contents();
   if (room_chr != STAIRS_DOWN) {
      put_message(_T("It's hard to climb the walls...FIND SOME STAIRS!!"));
      return 1 ;
   }

   bool prev_level_known = level_known[player.level] ;
   if (player.level < 7) {
      player.level++ ;
      // if (location_forgotten) {
      //    if (level_known[player.level])
      //       location_forgotten = false ;
      // } else {
      //    level_known[player.level] = true ;
      // }
      if (prev_level_known) {
         level_known[player.level] = true ;
      }
      draw_all_room_sprites() ;  //  formerly Map()/show_location()
      update_room() ;
      react_to_room(NULL) ;
      put_message(_T("Here you find stairs going up.")); 
   }
   Comments() ;
   return 0;
}

//****************************************************************************
int move_up(HWND hwnd)
{
   int room_chr = get_room_contents();
   if (room_chr != STAIRS_UP) {
      put_message(_T("It's hard to climb the walls...FIND SOME STAIRS!!"));
      return 1 ;
   }

   if (player.level > 0) {
      player.level-- ;
      //  move-UP should not update the "level known" flag,
      //  only move-DOWN should do so!!
      // if (location_forgotten) {
      //    if (level_known[player.level])
      //       location_forgotten = false ;
      // } else {
      //    level_known[player.level] = true ;
      // }
      draw_all_room_sprites() ;  //  formerly Map()/show_location()
      update_room() ;
      react_to_room(NULL) ;
      put_message(_T("Here you find stairs going down.")); 
   }
   Comments() ;
   return 0;
}

//*********************************************************
//  this returns a valid random direction
//*********************************************************
typedef enum key_dirs_e {
Key_N = 0,
Key_E,
Key_W,
Key_S 
} key_dirs_t ;

static key_dirs_t get_random_direction(void)
{
   while (1) {
      switch (random(4)) {
      case Key_N:  //  north
         if (player.y > 0) 
            return Key_N;
         break;

      case Key_E:  //  east
         if (player.x < 7) 
            return Key_E;
         break;

      case Key_W:  //  west
         if (player.x > 0) 
            return Key_W;
         break;

      case Key_S:  //  south
         if (player.y < 7) 
            return Key_S;
         break;
      }  //lint !e744  no default
   }  //  infinite loop
}

//****************************************************************************
void move_one_square(HWND hwnd)
{
   switch (get_random_direction()) {
   case Key_N:  move_north(hwnd) ; break;
   case Key_E:  move_east (hwnd) ; break;
   case Key_W:  move_west (hwnd) ; break;
   case Key_S:  move_south(hwnd) ; break;
   }  //lint !e744  no default
}

//****************************************************************************
int look_in_direction(HWND hwnd, unsigned key)
{
   int contents ;
   TCHAR *sptr ;

   switch (key) {
   case kw:
   case kLEFT:
      if (player.x == 0) {
         put_message(_T("you see a wall to the west")) ;
      } else {
         mark_room_as_known(player.x-1, player.y, player.level) ;
         contents = get_room_contents(player.x-1, player.y, player.level) ;
         // if (contents >= MONSTER_BASE  &&  contents <= MONSTER_END) {
         if (is_monster_index(contents)) {
            sptr = get_object_name(contents) ;
            _stprintf(tempstr, _T("The Lamp shines west.  There you see %s %s [ L%u ]"), 
               // starts_with_vowel(sptr) ? _T("an") : _T("a"), sptr,
               get_monster_prefix(sptr), sptr,
               // get_object_name(contents), 
               contents - MONSTER_BASE + 1) ;
         } else {
            _stprintf(tempstr, _T("The Lamp shines west.  There you see %s"), 
               get_object_name(contents)) ;
         }
         put_message(tempstr) ;
      }
      break;

   case kn:
   case kUP:
      if (player.y == 0) {
         put_message(_T("you see a wall to the north")) ;
      } else {
         mark_room_as_known(player.x, player.y-1, player.level) ;
         contents = get_room_contents(player.x, player.y-1, player.level) ;
         // if (contents >= MONSTER_BASE  &&  contents <= MONSTER_END) {
         if (is_monster_index(contents)) {
            sptr = get_object_name(contents) ;
            _stprintf(tempstr, _T("The Lamp shines north.  There you see %s %s [ L%u ]"), 
               // starts_with_vowel(sptr) ? _T("an") : _T("a"), sptr,
               get_monster_prefix(sptr), sptr,
               contents - MONSTER_BASE + 1) ;
         } else {
            _stprintf(tempstr, _T("The Lamp shines north.  There you see %s"), 
               get_object_name(contents)) ;
         }
         put_message(tempstr) ;
      }
      break;

   case ke:
   case kRIGHT: 
      if (player.x == 7) {
         put_message(_T("you see a wall to the east"))  ;
      } else {
         mark_room_as_known(player.x+1, player.y, player.level) ;
         contents = get_room_contents(player.x+1, player.y, player.level) ;
         // if (contents >= MONSTER_BASE  &&  contents <= MONSTER_END) {
         if (is_monster_index(contents)) {
            sptr = get_object_name(contents) ;
            _stprintf(tempstr, _T("The Lamp shines east.  There you see %s %s [ L%u ]") , 
               // starts_with_vowel(sptr) ? _T("an") : _T("a"), sptr,
               get_monster_prefix(sptr), sptr,
               contents - MONSTER_BASE + 1) ;
         } else {
            _stprintf(tempstr, _T("The Lamp shines east.  There you see %s") , 
               get_object_name(contents)) ;
         }
         put_message(tempstr) ;
      }
      break;

   case ks:
   case kDOWN: //  look south
      if (player.y == 7) {
         put_message(_T("you see a wall to the south") ) ;
      } else {
         mark_room_as_known(player.x, player.y+1, player.level) ;
         contents = get_room_contents(player.x, player.y+1, player.level) ;
         // if (contents >= MONSTER_BASE  &&  contents <= MONSTER_END) {
         if (is_monster_index(contents)) {
            sptr = get_object_name(contents) ;
            _stprintf(tempstr, _T("The Lamp shines south.  There you see %s %s [ L%u ]") , 
               // starts_with_vowel(sptr) ? _T("an") : _T("a"), sptr,
               get_monster_prefix(sptr), sptr,
               contents - MONSTER_BASE + 1) ;
         } else {
            _stprintf(tempstr, _T("The Lamp shines south.  There you see %s") , 
               get_object_name(contents)) ;
         }
         put_message(tempstr) ;
      }
      break;

   default:
      put_message(_T("The Lamp shines in your eyes.  You are blinded!!") ) ;
      // player.is_blind = random(5) ;
      player.blind_count = 1 + random(5); 
      player.is_blind = 1 ;
      return 1 ;
   }
   return 0;
}  //lint !e715

//****************************************************************************
int light_a_flare(HWND hwnd)
{
   if (player.flares == 0) {
      put_message(_T("Hey, Bright One, you're out of flares!!")) ;
      return 1;
   }
   player.flares-- ;

   if (player.is_blind) {
      put_message(_T("Fat lotta good THAT did you, blind one...")) ;
      return 1 ;
   }

   HDC hdc = GetDC(hwndMapArea) ;
   // show_status(hdc) ;
   show_flares() ;

   unsigned xi = (player.x == 0) ? 0 : player.x-1 ;
   unsigned xf = (player.x == 7) ? 7 : player.x+1 ;
   unsigned yi = (player.y == 0) ? 0 : player.y-1 ;
   unsigned yf = (player.y == 7) ? 7 : player.y+1 ;
   unsigned x, y ;
   for (y=yi; y<=yf; y++) {
      for (x=xi; x<=xf; x++) {
         // mark_room_as_known(x, y, player.level) ;
         if (x != player.x  ||  y != player.y) {
            mark_room_as_known(x, y, player.level) ;
            draw_room_contents(hdc, x, y) ;
         }
      }
   }
   ReleaseDC(hwndMapArea, hdc) ;
   put_message(_T("Ahhhh... that's better...")) ;
   return 0;   
}

//****************************************************************************
static int gaze_into_orb(HWND hwnd)
{
   unsigned A, B, C ;

   if (player.is_blind) {
      _stprintf(tempstr, _T("You can't SEE, you foolish %s..."), race_str[player.race]) ;
      put_message(tempstr) ;
      return 1;
   }

   switch (random(6)) {
   case 0:
      put_message(_T("You see yourself in a bloody heap."));
      if (player.has_runestaff) 
         A = 1 + random(9);
      else if (player.has_orb_of_Zot)  
         A = 1 + random(15);
      else
         A = 1 + random(3); 

      if (player.hit_points <= A) {
         player.hit_points = 0 ;
         player_dies(hwnd) ;
      } else {
         player.hit_points -= A ;
         show_hit_points() ;
      }
      break;

   case 1: 
      _stprintf(tempstr, _T("You see yourself drinking from a pool and becoming a %s"),
            get_object_name(MONSTER_BASE + random(12))) ;
      put_message(tempstr) ;
      break;

   case 2: 
      _stprintf(tempstr, _T("You see a %s looking at you speculatively."),
         get_object_name(MONSTER_BASE + random(12))) ;
      put_message(tempstr) ;
      break;

   case 3: 
      put_message(WIN_GREEN, _T("You see your girlfriend with a green squirrel."));
      break;

   case 4: 
      A = random(DIMEN_COUNT); B = random(DIMEN_COUNT); C = random(DIMEN_COUNT); mark_room_as_known(A, B, C);
      _stprintf(tempstr, _T("You see %s at (%d,%d), level %d."),
         get_object_in_room(A, B, C), A, B, C) ;
      put_message(tempstr) ;
      break;

   case 5: 
      if (player.has_orb_of_Zot) {
         _stprintf(tempstr, _T("You see your image looking back at you quizically...")) ;
      } else 
      if (player.has_runestaff) {
         if (random(10) < 5) {
            A = orb_room.x; B = orb_room.y; C = orb_room.level ;
         } else {
            A = random(DIMEN_COUNT); B = random(DIMEN_COUNT); C = random(DIMEN_COUNT);
         }
         _stprintf(tempstr, _T("You see the ** ORB OF ZOT ** at (%d,%d), Level %d!!"),
            A, B, C);
      } else 
      {
         _stprintf(tempstr, _T("You see %s carrying a large, ornate staff."),
            get_object_in_room(runestaff_room.x, runestaff_room.y, runestaff_room.level)) ;
      }
      put_message(tempstr) ;
      break;
   }  //lint !e744  no default
   return 0;   
}

//****************************************************************************
static int open_book_or_chest(HWND hwnd, int contents)
{
   unsigned Q, room ;

   //*****************************************************
   //  deal with a book
   //*****************************************************
   if (contents == BOOK) {
      if (player.is_blind) {
         put_message(_T("The book laughs, and jumps from your hands")) ;
         return 1;
      }

      // unsigned opts = random(6) ;
      // opts = 0 ;  //@@@  DEBUG
      // switch (opts) {
      switch (random(7)) {
      case 0:
         if (player.treasures[TR_OPAL_EYE]) {
            put_message(_T("You open the book and   *** FLASH!! ***"));
            put_message(_T(" ")) ;
            put_message(_T("You are blinded for a few moments...")) ;
            put_message(_T("then the aura of the Opal Eye flows over you")) ;
            put_message(_T("and your sight is restored!!  (Whew)")) ;
            
         } else {
            player.blind_count = 1 + random(15); 
            player.is_blind = 1 ;
            draw_main_screen(NULL); //  newly blinded
            put_message(_T("You open the book and   *** FLASH!! ***"));
            _stprintf(tempstr, _T("OH NO! YOU ARE NOW A BLIND %s !!"), race_str[player.race]);
            put_message(tempstr) ;
         }
         break;

      case 1:
         put_message(_T("You open the book and find that"));
         put_message(_T("IT'S ANOTHER VOLUME OF ZOT'S POETRY! - YECH!!"));
         break;

      case 2:
         _stprintf(tempstr, _T("You open the book and find an old copy of Play%s!"), race_str[random(4)]);
         put_message(tempstr) ;
         break;

      case 3:
         put_message(_T("You open a MANUAL OF INTELLIGENCE!")); 
         player.iq = 18 ;
         show_int() ;
         break;

      case 4:
         put_message(_T("You open a MANUAL OF DEXTERITY!")); 
         player.dex = 18 ;
         show_dex() ;
         break;

      case 5:
         put_message(_T("You open a MANUAL OF STRENGTH! ")); 
         player.str = 18 ;
         adjust_hit_points() ;
         show_str() ;
         show_hit_points() ;
         break;

      case 6:
         if (player.treasures[TR_BLUE_FLAME]) {
            put_message(_T("THE BOOK STICKS TO YOUR HANDS -"));
            put_message(_T("but the BLUE FLAME dissolves the glue... Whew!!")) ;
         } else {
            put_message(_T("THE BOOK STICKS TO YOUR HANDS -"));
            put_message(_T("NOW YOU ARE UNABLE TO DRAW YOUR WEAPON!"));
            // player.weapon = 4 ;
            player.book_count = 1 + random(15); 
            player.has_book = true ;
            show_weapon() ;
         }
         break;
      }  //lint !e744  no default

      clear_room(); 
   }  //  if contents == BOOK 
   //*********************************************************************
   //  deal with a chest
   //  Let's change the rules here a little...
   //  Rather than having an equal chance of each of the alternatives,
   //  which discouranges people from using the chests,
   //  let's increase the probability of getting cash...
   //    40% cash
   //    30% confusion
   //    20% explosion
   //    10% smoky room
   //*********************************************************************
   else if (contents == CHEST) {
      room = random(10) ;
      if (room < 4) {
         Q=random(1000);
         _stprintf(tempstr, _T("YOU OPEN THE CHEST AND  FIND %d GOLD PIECES!"), Q) ; 
         put_message(tempstr) ;
         player.gold += Q; 
         show_gold() ;
         clear_room(); 
      } else if (room < 7) {
         put_message(_T("You open the chest and find  **GAS** !!")) ;
         put_message(_T("YOU STAGGER FROM THE ROOM IN CONFUSION!!")) ; 
         ScrambleAttr() ; 
         move_one_square(hwnd) ;
         //  don't remove chest in this case
         // clear_room(); 
      } else if (room < 9) {
         put_message(_T("KABOOM!  THE CHEST EXPLODES!!"));
         Q = random(6); 
         put_color_msg(TERM_MONSTER_HIT, _T("You took %u points damage"), Q) ;
         if (Q >= player.hit_points) {
            player_dies(hwnd) ;
            return -1;
         } 
         player.hit_points -= Q ;
         show_hit_points() ;
         clear_room(); 
      } else {
         push_keymap(KEYMAP_ZOT_SCR) ;
         draw_zot_window(hwnd) ;
         clear_room(); 
      }
   } else {
      put_message(_T("You can call your pen, but it won't answer...")) ;
      return 1;
   }
   return 0;
}

//**********************************************************************
//  returns:
//    0 = normal exit
//    1 = decision to be made
//   -1 = player dies
//**********************************************************************
static int drink_from_pool(HWND hwnd)
{
   unsigned j ;

   switch (random(8)) {
   case 0:
      put_message(_T("You take a drink and feel stronger"));
      j = 1 + random(3) ;
      player.str = dmin(player.str + j, 18) ;
      adjust_hit_points() ;
      show_str() ;
      show_hit_points() ;
      break;

   case 1:
      put_message(_T("You take a drink and feel weaker"));
      j = 1 + random(3) ;
      player.str -= j ;
      if (player.str <= j) {
         player.str = 0 ;
         player_dies(hwnd) ;
         return -1 ;
      }
      adjust_hit_points() ;
      show_str() ;
      show_hit_points() ;
      break;

   case 2: 
      put_message(_T("You take a drink and feel smarter"));
      j = 1 + random(3) ;
      player.iq = dmin(player.iq + j, 18) ;
      show_int() ;
      break;

   case 3: 
      put_message(_T("You take a drink and feel dumber"));
      j = 1 + random(3) ;
      player.iq -= j ;
      if (player.iq <= j) {
         player.iq = 0 ;
         player_dies(hwnd) ;
         return -1 ;
      }
      show_int() ;
      break;

   case 4: 
      put_message(_T("You take a drink and feel nimbler"));
      j = 1 + random(3) ;
      player.dex = dmin(player.dex + j, 18) ;
      show_dex() ;
      break;

   case 5: 
      put_message(_T("You take a drink and feel clumsier"));
      j = 1 + random(3) ;
      if (player.dex <= j) {
         player.dex = 0 ;
         player_dies(hwnd) ;
         return -1 ;
      }
      player.dex -= j ;
      show_dex() ;
      break;

   case 6: 
      while (1) {
         j = random(4); 
         if (j != player.race) {
            player.race = j ;
            _stprintf(tempstr, _T("With a scream of agony, you turn into a %s"), race_str[j]) ;
            put_message(tempstr) ;
            //  alter stats when this happens??
            break;
         }
      }
      break;

   case 7: 
      if (!player.has_orb_of_Zot) {
         put_message(_T("An image forms in the pool..... At first it looks"));
         put_message(_T("like a door into a forest, but quickly fades into"));
         put_message(_T("a reflection of your face."));
         break;
      } 
      put_message(_T(" "));
      put_message(_T(" "));
      put_message(_T("An image of a door shimmers in the water."));
      put_message(_T("through the door, you see a forest with a cool stream"));
      put_message(_T("flowing through it..."));
      put_message(_T("WHICH DIRECTION WILL YOU GO? ? "));
      return 1 ; //  decision to be made
      // break;
   }  //lint !e744  no default

   return 0;
}

//****************************************************************************
int execute_local_object(HWND hwnd)
{
   int result = 1 ;
   int contents = get_room_contents();
   switch (contents) {
   case CRYSTAL_ORB:
      result = gaze_into_orb(hwnd);
      break ;
      
   case POOL:
      result = drink_from_pool(hwnd); 
      if (result < 0) 
         return 1;
      if (result > 0) 
         push_keymap(KEYMAP_POOLDIR) ;
      break ;
      
   case BOOK:
   case CHEST:
      result = open_book_or_chest(hwnd, contents); 
      break ;
   
   case VENDOR:   
      result = trade_with_vendor(hwnd);  
      break ;
      
   default:
      break ;
   }
   return result ;
}
      
//****************************************************************************
int teleport(HWND hwnd, unsigned inchr)
{
   static unsigned x, y, level ;
   static unsigned state = 0 ;

   keymap_show_state();
   switch (state) {
   case 0:
      if (!player.has_runestaff) {
         put_message(_T("If you need to pee, look for a bush!!"));
         return 1;
      }
      push_keymap(KEYMAP_TELEPORT) ;
      put_message(_T("[ ?, ?, ? ]: Enter X coord:")) ;
      state = 1 ;
      break;

   case 1:
      if (inchr < k0  ||  inchr > k7) {
         put_message(_T("yeah, right...")) ;
         put_message(_T("[ ?, ?, ? ]: Enter X coord:")) ;
         // state = 0 ;
         // push_keymap(KEYMAP_DEFAULT) ;
         // pop_keymap() ;
         break;
      }
      x = inchr - 0x30 ;
      _stprintf(tempstr, _T("[ %u, ?, ? ]: Enter Y coord:"), x) ;
      put_message(tempstr) ;
      state = 2 ;
      break;

   case 2:
      if (inchr < k0  ||  inchr > k7) {
         put_message(_T("yeah, right...")) ;
         _stprintf(tempstr, _T("[ %u, ?, ? ]: Enter Y coord:"), x) ;
         put_message(tempstr) ;
         // state = 0 ;
         // pop_keymap() ;
         break;
      }
      y = inchr - 0x30 ;
      _stprintf(tempstr, _T("[ %u, %u, ? ]: Enter Z coord:"), x, y) ;
      put_message(tempstr) ;
      state = 3 ;
      break;

   case 3:
      if (inchr < k0  ||  inchr > k7) {
         put_message(_T("yeah, right...")) ;
         _stprintf(tempstr, _T("[ %u, %u, ? ]: Enter Z coord:"), x, y) ;
         put_message(tempstr) ;
         // state = 0 ;
         // pop_keymap() ;
         break;
      }
      level = inchr - 0x30 ;

      //  restore the source room
      HDC hdc = GetDC(hwndMapArea) ;
      restore_room(hdc) ;
      ReleaseDC(hwndMapArea, hdc) ;

      //  move the player
      player.x = x ;
      player.y = y ;
      if (level != (unsigned) player.level) {
         player.level = level ;
         draw_main_screen(NULL) ;   //  teleport to new location
      } else {
         player.level = level ;
      }
      //  react to new room
      update_room() ;
      // react_to_room(hwnd) ;
      
      if (player.x     == orb_room.x  &&
          player.y     == orb_room.y  &&
          player.level == orb_room.level) {
         put_color_msg(TERM_RUNESTAFF, _T("GREAT UNMITIGATED ZOT!"));
         put_color_msg(TERM_RUNESTAFF, _T(" "));
         put_color_msg(TERM_RUNESTAFF, _T("You just found  *** THE ORB OF ZOT *** !"));
         put_color_msg(TERM_RUNESTAFF, _T(" "));
         put_color_msg(TERM_RUNESTAFF, _T("The RuneStaff has disappeared..."));
         player.has_runestaff = false ;
         player.has_orb_of_Zot = true ;
         show_treasures() ;
         pop_keymap() ; //  remove current TELEPORT keymap
         // draw_main_screen(NULL) ;   //  teleported to orb of zot
      } else {
         pop_keymap() ; //  remove current TELEPORT keymap
         react_to_room(NULL) ;
         // reset_keymap(KEYMAP_DEFAULT);
      }
      state = 0 ;
      break;
   }  //lint !e744  no default

   return 0;
}

//****************************************************************************
extern int vendor_angry ;  //  combat.cpp

int attack_vendor(HWND hwnd)
{
   int contents = get_room_contents() ;
   if (contents != VENDOR) {
      // put_message(_T("***  You're sure attacky person! (no monster found)")) ;
      return 0;
   }
   vendor_angry = 1 ;
   react_to_monsters(NULL);  
   return 0;
}

//*************************************************************
void adjust_hit_points(void)
{
   player.hit_points = 2 * player.str ;
}

//*************************************************************
void view_special_items(void)
{
   unsigned j ;

   infoout(_T("you are currently exploring %s"), names[player.castle_nbr].c_str()) ;

   infoout(_T("you possess:")) ;
   set_term_attr_default() ;
   if (player.treasure_count == 0) {
      // SendMessage(special_fields[idx], WM_SETFONT, (WPARAM) hfont_comic_sans_bold, MAKELPARAM(FALSE, 0)); 
      put_message(_T("Your miserable life")) ;
   } else {
      for (j=0; j<8; j++) {
         if (player.treasures[j]) {
            // SendMessage(special_fields[idx], WM_SETFONT, (WPARAM) hfont_comic_sans_normal, MAKELPARAM(FALSE, 0)); 
            put_message(get_object_name(TREASURE_BASE+j)) ;
         }
      }
   }
   if (player.has_runestaff) {
      put_message(_T(" ")) ;
      put_message(WIN_GREEN, _T("the RuneStaff")) ;
   } else 
   if (player.has_orb_of_Zot) {
      put_message(_T(" ")) ;
      put_message(WIN_MAGENTA, _T("the Orb of Zot !!")) ;
   }

   if (player.curse_flags) {
      put_message(_T(" ")) ;
      infoout(_T("you are afflicted by:")) ;
      unsigned tmask = 1 ;
      for (j=0; j<3; j++) {
         if (player.curse_flags & tmask) {
            // SendMessage(special_fields[idx], WM_SETFONT, (WPARAM) hfont_comic_sans_bold, MAKELPARAM(FALSE, 0)); 
            put_message(WIN_RED, curse_str[j].c_str()) ;
         }
         tmask <<= 1 ;
      }
   }

   if (player.book_count > 0) 
      queryout(_T("book count=%d"), player.book_count) ;
   if (player.blind_count > 0) 
      queryout(_T("blind count=%d"), player.blind_count) ;
   set_term_attr_default() ;
}

//*************************************************************
static TCHAR status_update_text[40] ;

static void show_str(void)
{
   _stprintf(status_update_text, _T(" %u"), player.str) ; 
   SetWindowText(status_windows[0], status_update_text) ;
}

void show_dex(void)
{
   _stprintf(status_update_text, _T(" %u"), player.dex) ; 
   SetWindowText(status_windows[1], status_update_text) ;
}
   
void show_int(void)
{
   _stprintf(status_update_text, _T(" %u"), player.iq ) ; 
   SetWindowText(status_windows[2], status_update_text) ;
}
   
void show_hit_points(void)
{
   _stprintf(status_update_text, _T(" %u"), player.hit_points) ; 
   SetWindowText(status_windows[3], status_update_text) ;
}
   
void show_weapon(void) 
{
   _stprintf(status_update_text, _T(" %s"), 
      (player.has_book) ? _T("A Book") : weapon_str[player.weapon].c_str()) ; 
   SetWindowText(status_windows[4], status_update_text) ;
}
   
void show_armour(void)
{
   _stprintf(status_update_text, _T(" %s (%u)"), armour_str[player.armour].c_str(), player.armour_points) ; 
   SetWindowText(status_windows[5], status_update_text) ;
}
   
static void show_flares(void)
{
   _stprintf(status_update_text, _T(" %u"), player.flares) ; 
   SetWindowText(status_windows[6], status_update_text) ;
}
   
void show_gold(void)
{
   _stprintf(status_update_text, _T(" %u"), player.gold) ; 
   SetWindowText(status_windows[7], status_update_text) ;
}
   
void show_treasures(void)
{
   //  draw all the treasures
   uint idx ;
   for (idx=0; idx<8; idx++) {
      if (player.treasures[idx]) {
         draw_sprite_treasure(
            object_data[TREASURE_BASE+idx].sprite_col,
            object_data[TREASURE_BASE+idx].sprite_row,
            idx) ;
      }
   }
   if (player.has_runestaff) {
      draw_sprite_treasure(
         object_data[RUNESTAFF].sprite_col,
         object_data[RUNESTAFF].sprite_row,
         8) ;
   } 
   else {
      clear_treasure_area(hwndTreasures[8], GetSysColor(COLOR_3DFACE));
   }
   if (player.has_orb_of_Zot) {
      draw_sprite_treasure(
         object_data[ORB_OF_ZOT].sprite_col,
         object_data[ORB_OF_ZOT].sprite_row,
         9) ;
   } 
   else {
      clear_treasure_area(hwndTreasures[9], GetSysColor(COLOR_3DFACE));
   }
}

//*************************************************************
void update_status(void)
{
   show_str() ;
   show_dex() ;
   show_int() ;
   show_hit_points() ;
   show_weapon() ;
   show_armour() ;
   show_flares() ;
   show_gold() ;
   show_treasures() ;
}

//*************************************************************
static void react_to_room(HWND hwndUnused)
{
   int contents ;
   unsigned Q ;
   unsigned tmask ;

   HDC hdc = GetDC(hwndMapArea) ;
   //  check for special contents
   CheckCurses(hdc) ;

   //  repeat Reacts until yesno;  
   contents = get_room_contents() ;

   switch (contents) {
   case GOLD_PIECES:
      clear_room(); 
      Q = 1 + random(200); 
      player.gold += Q ;
      _stprintf(tempstr, _T("You count %u coins in the pile!"), Q) ;
      put_message(tempstr) ;
      show_gold() ;
      break;

   case FLARES:
      clear_room(); 
      Q = 1 + random(5) ;
      player.flares += Q ;
      _stprintf(tempstr, _T("You find %u flares lying here!"), Q) ;
      show_flares() ;
      break;

   case WARP:
      if (!player.is_blind)
         draw_char_cursor(hdc, OFF) ;
      player.x=random(DIMEN_COUNT); player.y=random(DIMEN_COUNT); player.level=random(DIMEN_COUNT);
      if (!player.is_blind) {
         draw_main_screen(NULL) ;   //  warp
      }
      update_room() ;
      ReleaseDC(hwndMapArea, hdc) ; //  release the hdc before recursive call
      react_to_room(NULL) ;
      break;

   case SINKHOLE:
      if (++player.level > 7)   player.level = 0 ;
      // location_forgotten = !level_known[player.level] ;
      if (!player.is_blind) {
         draw_main_screen(NULL) ;   //  sinkhole
      }
      update_room() ;
      ReleaseDC(hwndMapArea, hdc) ; //  release the hdc before recursive call
      react_to_room(NULL) ;
      break;

   case VENDOR: 
      if (vendor_angry) {
         // update_room(hdc) ;
         react_to_monsters(hdc);  
      }
      break;

   case KOBOLD   : 
   case ORC      : 
   case KRESH    : 
   case GOBLIN   : 
   case OGRE     : 
   case TROLL    : 
   case MINOTAUR : 
   case UR_VILE  : 
   case GARGOYLE : 
   case CHIMERA  : 
   case BALROG   : 
   case DRAGON   : 
      // update_room(hdc) ;
      react_to_monsters(hdc);  
      break ;

   case SILMARIL   : 
   case PALANTIR   : 
   case BLUE_FLAME : 
   case GREEN_GEM  : 
   case OPAL_EYE   : 
   case PALE_PEARL : 
   case NORN_STONE : 
   case RUBY_RED   : 
      clear_room(); 
      // tmask = 1 << (contents - RUBY_RED) ;
      tmask = contents - TREASURE_BASE ;
      _stprintf(tempstr, _T("You find %s [%u]... It's now yours!!"), 
         get_object_name(contents), contents) ;
      put_message(tempstr) ;
      player.treasure_count++ ;
      player.treasures[tmask] = 1 ;
      show_treasures() ;

      //  see if anything helped with anything
// ¦        RUBY RED   - AVOID LETHARGY     PALE PEARL - AVOID LEECH              ¦
// ¦        GREEN GEM  - AVOID FORGETTING   OPAL EYE   - CURES BLINDNESS          ¦
// ¦        BLUE FLAME - DISSOLVES BOOKS    NORN STONE - SNAKE ANTIDOTE?          ¦
// ¦        PALANTIR   - NO BENEFIT         SILMARIL   - NO BENEFIT               ¦
      switch (contents) {
      case BLUE_FLAME:
         if (player.has_book) {
            put_message(_T("The Blue Flame dissolves the book in your hands!!")) ;
            player.book_count = 0 ;
            player.has_book = false ;
            show_weapon() ;
         }
         break;

      case OPAL_EYE:
         if (player.is_blind) {
            put_message(_T("The Opal Eye cures your blindness!!")) ;
            player.blind_count = 0 ;
            player.is_blind = false ;
            draw_main_screen(NULL) ;   //  opal eye cures blindness
            update_position() ;
         }
         break;

      case RUBY_RED:
         if (player.curse_flags & CR_LETHARGY) {
            player.curse_flags &= ~CR_LETHARGY ;
            put_message(_T("The Ruby Red cures the Curse of Lethargy...")) ;
         }
         break;

      case PALE_PEARL:
         if (player.curse_flags & CR_LEECH) {
            player.curse_flags &= ~CR_LEECH ;
            put_message(_T("The Pale Pearl cures the Curse of the Leech...")) ;
         }
         break;

      case GREEN_GEM:
         if (player.curse_flags & CR_FORGET) {
            player.curse_flags &= ~CR_FORGET ;
            put_message(_T("The Green Gem cures the Curse of Forgetfulness...")) ;
         }
         break;
      }  //lint !e744  no default
      break;

   default: break ;
   }  //  switch on room contents
   ReleaseDC(hwndMapArea, hdc) ;
}  //lint !e715  Symbol 'hwndUnused' (line 1667) not referenced

//**********************************************************************
static void update_blind_status(void)
{
   if (player.is_blind) {
      if (--player.blind_count == 0) {
         player.is_blind = false ; 
         draw_main_screen(NULL) ;   //  recover from blindness
         update_position() ;
      }
   }
}   

//**********************************************************************
static void update_book_status(void)
{
   if (player.has_book) {
      if (--player.book_count == 0) {
         player.has_book = false ; 
         update_status() ;
      }
   } 
}   

//*************************************************************
void do_idle_tasks(void)
{
   update_blind_status() ;
   update_book_status() ;
}

//*************************************************************
void draw_beginning_screen(void)
{
   draw_main_screen(NULL) ;   //  draw_beginning_screen()
   term_set_font(_T("Bodacious-Normal"), 120, EZ_ATTR_NORMAL) ;
   set_term_attr_default();
   update_room() ;
   level_known[0] = true ;
}

//*************************************************************
void set_up_working_spaces(HWND hwnd)
{
   status_windows[0] = GetDlgItem(hwnd, IDC_STR) ;
   status_windows[1] = GetDlgItem(hwnd, IDC_DEX) ;
   status_windows[2] = GetDlgItem(hwnd, IDC_INT   ) ;
   status_windows[3] = GetDlgItem(hwnd, IDC_HP    ) ;
   status_windows[4] = GetDlgItem(hwnd, IDC_WEAPON) ;
   status_windows[5] = GetDlgItem(hwnd, IDC_ARMOUR) ;
   status_windows[6] = GetDlgItem(hwnd, IDC_FLARES) ;
   status_windows[7] = GetDlgItem(hwnd, IDC_GP    ) ;

   hwndMapArea = GetDlgItem(hwnd, IDC_MAP_AREA) ;
   SetWindowPos(hwndMapArea, HWND_TOP, 0, 0, IMAGE_WIDTH+4, IMAGE_HEIGHT+4, SWP_NOMOVE);

   hwndTreasures[0] = GetDlgItem(hwnd, IDC_T0) ;
   hwndTreasures[1] = GetDlgItem(hwnd, IDC_T1) ;
   hwndTreasures[2] = GetDlgItem(hwnd, IDC_T2) ;
   hwndTreasures[3] = GetDlgItem(hwnd, IDC_T3) ;
   hwndTreasures[4] = GetDlgItem(hwnd, IDC_T4) ;
   hwndTreasures[5] = GetDlgItem(hwnd, IDC_T5) ;
   hwndTreasures[6] = GetDlgItem(hwnd, IDC_T6) ;
   hwndTreasures[7] = GetDlgItem(hwnd, IDC_T7) ;
   hwndTreasures[8] = GetDlgItem(hwnd, IDC_RS) ;
   hwndTreasures[9] = GetDlgItem(hwnd, IDC_OZ) ;

   //  resize the treasure frames
   uint idx ;
   for (idx=0; idx<10; idx++) {
      //  the +4 addition to size, allows room for the frame
      SetWindowPos(hwndTreasures[idx], NULL, 0, 0, SPRITE_WIDTH+4, SPRITE_HEIGHT+4, SWP_NOMOVE);
   }
}

//****************************************************************************
// static TCHAR *intro_msg[] = {
static std::vector<std::wstring> intro_msg {
L"MANY CYCLES AGO, IN THE KINGDOM OF N'DIC,  THE GNOMIC",
L"WIZARD ZOT FORGED HIS GREAT *ORB OF POWER*.  HE SOON",
L"VANISHED, LEAVING BEHIND HIS VAST SUBTERRANEAN CASTLE",
L"FILLED WITH ESURIENT MONSTERS, FABULOUS TREASURES,",
L"AND THE INCREDIBLE *ORB OF ZOT*.  FROM THAT TIME HENCE,",
L"MANY A BOLD YOUTH HAS VENTURED INTO THE WIZARD'S CASTLE.",
L"AS OF NOW, *NONE* HAS EVER EMERGED VICTORIOUSLY!  BEWARE!!"
// ,0 //  the NULL-terminator doesn't work with vector class
};

// unsigned unused_lint_separator2138 = 0 ;  //lint !e19 !e129

void draw_intro_screen(HWND hwnd)
{
   put_message(WIN_MAGENTA, L"* * * THE WIZARD'S CASTLE * * *") ;
   // for (auto j=0U; intro_msg[j] != 0; j++) { //  array version
   for (auto j=0U; j < intro_msg.size() ; j++) {   //lint !e737
   // for (auto it = intro_msg.begin(); it != intro_msg.end(); ++it) {
      put_message(WIN_BLUE, (wchar_t *) intro_msg[j].c_str()) ;
      // put_message(WIN_BLUE, it->c_str()) ;
   }
   infoout(_T("You are now entering %s"), names[player.castle_nbr].c_str()) ;
   queryout(_T("Do you wish to select your own attributes  (Type 's'),")) ;
   queryout(_T("or use the ones that the derelict prefers? (Type 'd')")) ;
}  // lint !e533

