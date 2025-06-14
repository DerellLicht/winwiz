//************************************************************************
//  Copyright (c) 1986-2025  Daniel D Miller
//  winwiz.exe - Wizard's Castle
//  keyboard.cpp - keyboard state-machine interface for WinWiz
//                         
//  01/03/11 14:21 - separated from main keyboard handler
//************************************************************************

//****************************************************************************

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "common.h"
#include "wizard.h"
#include "keywin32.h"

//lint -esym(769, keymap_states_e::KEYMAP_SPELL)

//  term_app.cpp
extern uint key_mask ;

//  from wfuncs.cpp
extern void do_idle_tasks(void);
extern int execute_local_object(HWND hwnd);
// extern int  open_book_or_chest(HWND hwnd);
// extern int  gaze_into_orb(HWND hwnd);
// extern int  drink_from_pool(HWND hwnd);
extern int  manage_zot_input(HWND hwnd, unsigned inchr);
extern int  teleport(HWND hwnd, unsigned inchr);
extern int  attack_vendor(HWND hwnd);
extern int  light_a_flare(HWND hwnd);
extern void view_special_items(void);

//  initscrn.cpp
extern void draw_init_screen(HWND hwnd);

//  combat.cpp
extern int  cast_spell(HWND hwnd, unsigned inchr);
extern int  run_one_encounter_round(HWND hwnd, unsigned inchr);

//*************************************************************
//  since Windows programs cannot just call getch() to get
//  a user input, we need a trickier method to allocate the
//  keyboard for specific purposes at any given time.
//  
//  The method that I'm using is to define "key maps",
//  which should more properly be called "keyboard states".
//  Each function which needs to use the keyboard will
//  be allocated a special keymap; when it is done, it will
//  need to remember to pop the keymap...
//*************************************************************
static keymap_states_t keymap_stack[MAX_KEYMAP_STATES] ;
static uint keymap_idx = 0 ;
static keymap_states_t keymap = KEYMAP_INTRO ;

//  note that push_keymap(KEYMAP_DEFAULT) is usually
int push_keymap(keymap_states_t new_keymap)
{
   if (keymap_idx == ((uint) MAX_KEYMAP_STATES-1))
      return 0;
   keymap_stack[keymap_idx] = keymap ;
   keymap = new_keymap ;
   keymap_idx++ ;
   // _stprintf(tempstr, "+keymap [%u]=%s", keymap_idx, (uint) keymap) ;
   // put_message(tempstr) ;
   return 1;
}

//  note: nobody looks at return code
int pop_keymap(void)
{
   if (keymap_idx > 0)
   {
      keymap_idx-- ;
   }
   keymap = keymap_stack[keymap_idx] ;
   // _stprintf(tempstr, "-keymap [%u]=%u", keymap_idx, (uint) keymap) ;
   // put_message(tempstr) ;
   return 1;
}

//*******************************************************************************
//  This function will pop all from current stack, 
//  then initialize to specified state.
//  This will hopefull solve issue with sometimes having keyboard get into
//  unknown state and not recover.
//*******************************************************************************
//lint -esym(714, reset_keymap)
//lint -esym(759, reset_keymap)
//lint -esym(765, reset_keymap)
void reset_keymap(keymap_states_t new_keymap_state)
{
   keymap_idx = 0 ;
   push_keymap(new_keymap_state) ;
   pop_keymap() ;
}

//*************************************************************
void keymap_show_state(void)
{
   _stprintf(tempstr, _T("keymap state: idx: %u, keymap: %u/%u"), 
      keymap_idx, (uint) keymap_stack[keymap_idx], (uint) keymap);
   put_message(tempstr) ;
}

//*************************************************************
void set_default_keymap(void)
{
   keymap = KEYMAP_DEFAULT ;
}

//*************************************************************
bool is_default_keymap(void)
{
   return (keymap == KEYMAP_DEFAULT) ? true : false ;
}

//*************************************************************
uint get_keymap_state(void)
{
   return (uint) keymap ;
}

//*************************************************************
static int kbd_handler_pool(HWND hwnd, unsigned inchr)
{
   unsigned skip_keymap_pop = 0 ;
   switch (inchr) {
   case kUP:    move_north(hwnd) ; break;
   case kDOWN:  move_south(hwnd) ; break;
   case kRIGHT: move_east(hwnd) ; break;
   case kLEFT:  move_west(hwnd) ; break;
   case kd:     
      win_game(hwnd) ;
      skip_keymap_pop = 1 ;
      break;
   default:
      put_message(_T("Hmph... you're as stupid as you look...")) ;
      break;
   }
   if (!skip_keymap_pop)
      pop_keymap() ;
   return 0;
}

//*************************************************************
static int kbd_handler_intro(HWND hwnd, unsigned inchr)
{
   // syslog("keymap=INTRO, inchr=%02X, kd=%02X, ks=%02X", inchr, kd, ks) ;
   if (inchr == kd  ||  inchr == kD) {
      keymap = KEYMAP_DEFAULT ;
      draw_beginning_screen() ;
   } else if (inchr == ks  ||  inchr == kS) {
      keymap = KEYMAP_INITCHAR ;
      draw_init_screen(hwnd) ;
   } else if (inchr == kESC) {
      SendMessage(hwnd, WM_DESTROY, 0, 0) ;
   } else {
      _stprintf (tempstr, _T("PRESS=0x%04X"), inchr);
      status_message(tempstr);
   }
   return 0;
}

//*************************************************************
static bool can_I_see(void)
{
   TCHAR msgstr[81] ;
   if (player.is_blind) {
      _stprintf(msgstr, _T("YOU CAN'T SEE, YOU BLIND %s !!"), get_race_str());
      put_message(msgstr) ;
      return false;
   }
   if (!player.has_lamp) {
      put_message(_T("Dang, it's dim in here!!")) ;
      return false;
   }
   return true;
}

//*************************************************************
//  wfuncs.cpp
static int default_kbd_handler(HWND hwnd, unsigned inchr)
{
   // int result ;
   do_idle_tasks() ;

   // syslog("inchr=%02X, key_down=%X", inchr, VK_DOWN) ;
   switch (inchr) {
   case kUP:    
      if (key_mask & kShift) {
         if (!can_I_see())
            break;
         look_in_direction(hwnd, inchr) ;
      } else {
         move_north(hwnd) ; 
      }
      break;

   case kDOWN:  
      if (key_mask & kShift) {
         if (!can_I_see())
            break;
         look_in_direction(hwnd, inchr) ;
      } else {
         move_south(hwnd) ; 
      }
      break;

   case kRIGHT: 
      if (key_mask & kShift) {
         if (!can_I_see())
            break;
         look_in_direction(hwnd, inchr) ;
      } else {
         move_east(hwnd) ; 
      }
      break;

   case kLEFT:  
      if (key_mask & kShift) {
         if (!can_I_see())
            break;
         look_in_direction(hwnd, inchr) ;
      } else {
         move_west(hwnd) ; 
      }
      break;

   case ku: move_up(hwnd) ; break;
   case kd: move_down(hwnd) ; break;

   case kl:
      if (!can_I_see())
         break;
      put_message(_T(" ")); 
      put_message(_T("Where do you shine the lamp? ")); 
      push_keymap(KEYMAP_LOOKDIR) ;
      break;

   case kh:  
      show_help_menu();
      break;
      
   case kf:  light_a_flare(hwnd); break;
   
   //  10/26/23 - Change all 'use object at this location' operations
   //             to use 'e' key (Execute), then select based on current object.
   case ke:
      execute_local_object(hwnd);
      break ;
   
   // case kg: gaze_into_orb(hwnd); break;
   // case ko: open_book_or_chest(hwnd); break;
   // case kt: trade_with_vendor(hwnd);  break ;
   
   // case kr:  
   //    result = drink_from_pool(hwnd); 
   //    if (result < 0) 
   //       return 1;
   //    if (result > 0) 
   //       push_keymap(KEYMAP_POOLDIR) ;
   //    break;
   
   case kg: 
   case ko: 
   case kt: 
   case kr:  
      put_message(_T("This command has been replaced by [E]xecute...")); 
      break;

   case kp: teleport(hwnd, inchr); break;
   case kj: view_special_items(); break;
   case ka: attack_vendor(hwnd);  break ;

   // case kCc:   //  these don't work
   // case kESC:
   //    SendMessage(hwnd, WM_DESTROY, 0, 0) ;
   //    break;

   // case kQMark:   //  this doesn't work
   //    dump_level_knowledge();
   //    break;

   default:
      _stprintf (tempstr, _T("PRESS=0x%04X"), inchr);
      status_message(tempstr);
      return -1;
      // break;
   }
   return 0;
}

//*************************************************************
//*************************************************************
int process_keystroke(HWND hwnd, unsigned inchr)
{
   int result ;

   //*********************************************************
   //  Windows is passing lower-case form of characters,
   //  while keywin32.h recognizes upper-case forms.
   //  Just convert characters as necessary.
   //*********************************************************
   if (inchr >= 'a'  &&  inchr <= 'z') {
      inchr &= ~(0x20) ;   //  convert lower-case letters to upper case
   }
   
   switch (keymap) {
   case KEYMAP_LOOKDIR:
      look_in_direction(hwnd, inchr) ;
      set_default_keymap() ;
      break;

   case KEYMAP_POOLDIR:
      return kbd_handler_pool(hwnd, inchr) ;

   case KEYMAP_MONSTER:
      result = run_one_encounter_round(hwnd, inchr) ;
      if (result < 0) 
         return result ;
      else if (result == 0)
         pop_keymap() ;
      break;

   case KEYMAP_SPELL_SELECTION:
      return cast_spell(hwnd, inchr) ;  

   case KEYMAP_ZOT_SCR:
      result = manage_zot_input(hwnd, inchr) ;  
      if (result < 0) {
         push_keymap(KEYMAP_WAIT_END) ;
      } else if (result > 0) {
         // push_keymap(KEYMAP_DEFAULT) ;
         pop_keymap() ;
         clear_room();
         draw_main_screen(NULL) ;
      }
      break;

   //  wait for one more keystroke before terminating
   case KEYMAP_WAIT_END:
      SendMessage(hwnd, WM_DESTROY, 0, 0) ;
      break;

   //  wait for one more keystroke before terminating
   case KEYMAP_TELEPORT:
      teleport(hwnd, inchr) ;
      break;

   //  handle keystrokes for intro screen
   case KEYMAP_INTRO:
      return kbd_handler_intro(hwnd, inchr);

   //  main keyboard handler
   default:
      return default_kbd_handler(hwnd, inchr);
      // break;
   }  //  switch keymap
   return 0;
}

