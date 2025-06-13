//****************************************************
//  wizard.h - Declarations for Wizard's Castle
//  
//  Written by:   Daniel D. Miller
//****************************************************


//************************************************************
#define  OFF   0
#define  ON    1

//****************************************************************************
//  debug: message-reporting data 
//****************************************************************************

#define  DBG_WINMSGS       0x01
#define  DBG_FORCE_RSTAFF  0x02

extern uint dbg_flags ;

//************************************************************
//  player attributes
//************************************************************
struct player_info {
   unsigned castle_nbr{} ; //  index into castle-name table
   unsigned x{0}, y{3}, level{0} ;
   unsigned str{8}, dex{16}, iq{8} ;
   unsigned hit_points {8};
   unsigned race {0};
   unsigned alt_race {0};  //  used if player is changed into a monster
   // unsigned sex ; //  not really used anywhere, even in original
   unsigned armour{1}, armour_points{7}, weapon{3} ;
   unsigned flares {0};
   unsigned gold {0};

   int   turns {0};
   int   treasure_count {0};
   unsigned treasures[8] {};  //  bitmapped field

   bool  has_orb_of_Zot{false} ;
   bool  has_runestaff{false} ;
   bool  has_lamp{true} ;

   //  curses and afflictions
   unsigned curse_flags{0} ;  //  bitmapped field
   bool  is_blind{false} ;
   int   blind_count{0} ;
   bool  has_book{false} ;
   int   book_count{} ;
} ;

extern player_info player ;

//  treasure flags constants
#define  TR_RUBY_RED       0
#define  TR_NORN_STONE     1
#define  TR_PALE_PEARL     2
#define  TR_OPAL_EYE       3
#define  TR_GREEN_GEM      4
#define  TR_BLUE_FLAME     5
#define  TR_PALANTIR       6
#define  TR_SILMARIL       7

//  curse flags
#define  CR_LETHARGY       1
#define  CR_LEECH          2
#define  CR_FORGET         4

//  curse indices
#define  CURSE_OF_LETHARGY    0
#define  CURSE_OF_LEECH       1
#define  CURSE_OF_FORGET      2

//****************************************************
struct castle_room {
   unsigned x{}, y{}, level{} ;  //  only used for special rooms (curse/orb/runestaff)
   bool is_known{false} ;
   unsigned contents{} ;  //  index into object_info[]
} ;

#define  DIMEN_COUNT    8
extern castle_room castle[DIMEN_COUNT][DIMEN_COUNT][DIMEN_COUNT] ; //  Castle definitions
extern castle_room runestaff_room, orb_room ;
extern castle_room curse_rooms[3] ;

//  constants for referencing room contents.  
//  This must match display_objects_t display_objects[] 
#define  UNSEEN_ROOM        0
#define  EMPTY_ROOM         1
#define  CASTLE_ENTRANCE    2
#define  STAIRS_UP          3
#define  STAIRS_DOWN        4

#define  OBJECT_BASE        5
#define  POOL               5
#define  CHEST              6
#define  GOLD_PIECES        7
#define  FLARES             8
#define  WARP               9
#define  SINKHOLE          10
#define  CRYSTAL_ORB       11
#define  BOOK              12
#define  OBJECT_END        12

#define  MONSTER_BASE      13
#define  KOBOLD            13
#define  ORC               14
#define  KRESH             15
#define  GOBLIN            16
#define  OGRE              17
#define  TROLL             18
#define  MINOTAUR          19
#define  UR_VILE           20
#define  GARGOYLE          21
#define  CHIMERA           22
#define  BALROG            23
#define  DRAGON            24
#define  MONSTER_END       24

#define  VENDOR            25

#define  TREASURE_BASE     26
#define  RUBY_RED          26
#define  NORN_STONE        27
#define  PALE_PEARL        28
#define  OPAL_EYE          29
#define  GREEN_GEM         30
#define  BLUE_FLAME        31
#define  PALANTIR          32
#define  SILMARIL          33
#define  TREASURE_END      33

#define  RUNESTAFF         34
#define  ORB_OF_ZOT        35

// race_str   { L"Human", L"Dwarf", L"Hobbit", L"Elf" } ;
#define  PLAYER            36
#define  HUMAN             36
#define  DWARF             37
#define  HOBBIT            38
#define  ELF               39

#define  LAST_OBJECT       40

//**************************************************************
//  misc string arrays
//**************************************************************
extern TCHAR tempstr[128] ;

//**************************************************************
//  misc variables
//**************************************************************

//**************************************************************
//  function prototypes
//**************************************************************

//  winwiz.cpp
extern uint cxClient, cyClient ;

struct attrib_table {
   COLORREF fgnd ;
   COLORREF bgnd ;
} ;

#define  NUM_TERM_ATTR_ENTRIES   8
void status_message(TCHAR *msgstr);
void status_message(uint idx, TCHAR *msgstr);

//  indices for put_color_msg()
enum {
TERM_NORMAL = 0,
TERM_INFO,
TERM_QUERY,
TERM_PLAYER_HIT,
TERM_MONSTER_HIT,
TERM_RUNESTAFF,
TERM_DEATH,
TERM_ATMOSPHERE
} ;

void show_help_menu(void);
int  termout(const TCHAR *fmt, ...);
int  infoout(const TCHAR *fmt, ...);
int  queryout(const TCHAR *fmt, ...);
int  term_append(const TCHAR *fmt, ...);
int  wterm_replace(const TCHAR *fmt, ...);
void put_message(TCHAR *msgstr);
void put_message(COLORREF attr, TCHAR *msgstr);
int  put_message(COLORREF attr, const TCHAR *fmt, ...);
int  put_message(COLORREF fgnd, COLORREF bgnd, const TCHAR *fmt, ...);
int  put_color_msg(uint idx, const TCHAR *fmt, ...);

unsigned random(unsigned Q);

//****************************************************************************
void set_term_attr_default(void);

//  CastleInit.cpp
void init_player(void);
void init_castle_contents(void);

//  keyboard.cpp
typedef enum keymap_states_e {
KEYMAP_DEFAULT = 0,
KEYMAP_LOOKDIR,
KEYMAP_POOLDIR,
KEYMAP_MONSTER,
KEYMAP_SPELL,
KEYMAP_ZOT_SCR,
KEYMAP_WAIT_END,
KEYMAP_TELEPORT,
KEYMAP_INTRO,
KEYMAP_INITCHAR,
KEYMAP_SPELL_SELECTION,
MAX_KEYMAP_STATES 
} keymap_states_t ;

uint get_keymap_state(void);
void set_default_keymap(void);
bool is_default_keymap(void) ;
// bool is_intro_screen_active(void);
int push_keymap(keymap_states_t new_keymap);
int pop_keymap(void);
void reset_keymap(keymap_states_t new_keymap_state);
void keymap_show_state(void);
int process_keystroke(HWND hwnd, unsigned inchr);
TCHAR *get_monster_prefix(TCHAR *monster_name);

//  about.cpp
BOOL CmdAbout(HWND hwnd);

//  from wfuncs.cpp
wchar_t const * const get_race_str(void);
wchar_t const * const get_race_str(uint idx);
void set_race_str(uint idx, TCHAR *newstr);
unsigned get_castle_name_count(void);
void create_gdiplus_elements(void);
void set_up_working_spaces(HWND hwnd);
void draw_intro_screen(HWND hwnd);
int  move_south(HWND hwnd);
int  move_north(HWND hwnd);
int  move_east(HWND hwnd);
int  move_west(HWND hwnd);
int  move_down(HWND hwnd);
int  move_up(HWND hwnd);
void move_one_square(HWND hwnd);
int  look_in_direction(HWND hwnd, unsigned chr);
unsigned get_room_contents(void);
TCHAR *get_room_contents_str(void);
TCHAR *get_object_name(int index);
void show_player(void);
void player_dies(HWND hwnd);
void win_game(HWND hwnd);
void draw_main_screen(HDC hdcUnused);
void clear_room(void);
void adjust_hit_points(void);
void show_dex(void);
void show_int(void);
void show_hit_points(void);
void show_weapon(void);
void show_armour(void);
void show_gold(void);
void draw_beginning_screen(void);
void update_status(void);
void show_treasures(void);
void update_cursor(void);
void render_combat_bitmap(void);
void dump_level_knowledge(void);

//  vendor.cpp
int  trade_with_vendor(HWND hwnd);

//  from combat.cpp
void react_to_monsters(HDC hdcUnused);

//  loadhelp.cpp
void view_help_screen(HWND hwnd);

