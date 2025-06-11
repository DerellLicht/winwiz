//*******************************************************************
//  Copyright (c) 1985-2025  Daniel D Miller
//  combat.cpp - combat code for Wizard's Castle game
//  
//  Written by:   Daniel D. Miller
//*******************************************************************
//  Combat strategy rules
//  Note that these are *not* the rules used by the original game.
//  I've re-written them for (what I consider to be) better balance.
//  This still isn't very good; for one thing, there's nothing
//  in the original game to reflect player level!!
//  The only variables are dex, str, weapon type.
//  Note that in the original game, strength was hit points.
//  
//  Plus, as it turns out, I'm not particularly experienced
//  in designing balanced combat systems!!
//  
//  Monster hit points:   level + random(2 * level) + 1
//    L1 =  2 -  3
//    L8 =  9 - 24
//  
//  player chance to hit: 5 * (1+random(dex)) = (5 - 90 %)
//    Original method:
//    7590 IF DX >= FNA(20)+(3*BL) GOTO 7630
//                  (1 + random(20)) + (3 * player.blind)
//    - Independent of monster level
//    - On high dex, you *always* hit
//  
//  monster chance to resist:
//    (monster level * 6) + random(monster level * 6) = (6 - 95 %)
//       L1 =  6 - 12
//       L8 = 48 - 95
//    
//  player damage:
//    1 + random(weapon damage)
//    Dagger = 6
//    Mace   = 8
//    Sword  = 10
//  
//  monster chance to hit:
//    random(monster level * 11)
//  
//  player chance to resist:
//    random(4 * dex) = 4 - 72
//    leather = +4
//    chain   = +10
//    plate   = +20
//  
//  monster damage: monster level
//  
//*******************************************************************
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "common.h"
#include "wizard.h"
#include "keywin32.h"

struct monster_info_s {
   int index {};
   TCHAR *desc {nullptr};
   unsigned hitpoints {};
   unsigned level {};
   unsigned treasures[8] {};
   unsigned treasure_count {};
} ;

//  this doesn't solve the problem...
//  This struct is invalid once encounter is ended,
//  even if monster (or vendor) is still alive
static monster_info_s monster_info ;

static int web_turns = 0 ;

int vendor_angry = 0 ;

//lint -esym(551, weapon_damage, armour_resist) Symbol not accessed
//  weapon_str { L"Hands ",  L"Dagger",  L"Mace  ", L"Sword ", L"A Book" } ;
static unsigned weapon_damage[5] = { 0, 6,  8, 10, 0 } ;
static unsigned armour_resist[4] = { 0, 4, 10, 20 } ;

static TCHAR *meal[8] = { _T("sandwich"),_T("stew"), _T("soup"), _T("burger"), _T("roast"), _T("filet"), _T("taco"), _T("pie") } ;
//*************************************************************
static void EndEncounter(void)
{
   unsigned Q ;

   if (monster_info.index == VENDOR) {
      if (monster_info.hitpoints == 0) {
         put_message(_T(" ")) ;
         _stprintf(tempstr, _T("%s %s lies dead at your feet."), 
            // starts_with_vowel(monster_info.desc) ? _T("an") : _T("a"),
            get_monster_prefix(monster_info.desc), 
            monster_info.desc);
         put_message(tempstr) ;
         put_message(_T(" ")) ;
         put_message(_T("YOU GET ALL HIS WARES: "));
         put_message(_T("ARMOR PLATE, A SWORD, AND A LAMP."));
         put_message(_T("ONE POTION OF STRENGTH"));
         put_message(_T("ONE POTION OF INTELLIGENCE"));
         put_message(_T("ONE POTION OF DEXTERITY"));

         player.weapon = 3; 
         player.armour = 3; 
         player.armour_points = 21; 
         player.has_lamp = true ;
         player.str += random(6) ;
         if (player.str > 18)
             player.str = 18 ;
         adjust_hit_points() ;
         player.iq  += random(6) ;
         if (player.iq > 18)
             player.iq = 18 ;
         player.dex += random(6) ;
         if (player.dex > 18)
             player.dex = 18 ;

         //  HOW DO WE HANDLE TREASURES ON VENDORS/MONSTERS NOW??
         //  Should we still store them in the treasure_rooms[] array??

         // J = 0; for I = 1 to 8 do {
         //    if (treasure[I,1]=X) AND (treasure[I,2]=Y) AND (treasure[I,3]=Z)
         //       then {  treasure[I,4] = 1; INC(treasure_count); J = 1; } }
         // if J = 1 then writeln('YOU ALSO RECEIVE ALL OF HIS TREASURES ! !");
         put_message(_T(" ")) ;

         Q = random(1000); 
         _stprintf(tempstr, _T("You now get his hoard of %u gold_pieces's"), Q);
         player.gold += Q ;
         show_gold() ;
      }
   } 
   //  if you fought anything *other* than a vendor
   else {
      Q = random(1000); 
      player.gold += Q ;
      _stprintf(tempstr, _T("%s %s lies dead at your feet."), 
         // starts_with_vowel(monster_info.desc) ? _T("an") : _T("a"),
         get_monster_prefix(monster_info.desc), 
         monster_info.desc);
      put_message(tempstr) ;
      _stprintf(tempstr, _T("You now get his hoard of %u GP's"), Q);
      put_message(tempstr) ;
      show_gold() ;

      if (random(20) < 3) {
         _stprintf(tempstr, _T("You stop for a snack of %s %s"),
            monster_info.desc, meal[random(DIMEN_COUNT)]);
         put_message(tempstr) ;
      }

      if (player.x == runestaff_room.x  &&
          player.y == runestaff_room.y  &&
          player.level == runestaff_room.level) {
         player.has_runestaff = true ;
         put_color_msg(TERM_RUNESTAFF, _T("GREAT ZOT!! You've found the RUNESTAFF!!"));
         show_treasures() ;
      }
   }
   web_turns = 0 ;
   clear_room();
   draw_main_screen(NULL) ;   //  end of combat
}

//*************************************************************
static int monsters_turn(HWND hwnd)
{
   if (monster_info.hitpoints == 0) 
      return 0;

   unsigned monster_attack, player_resist, damage, apts ;

   switch (web_turns) {
   case 0:
      monster_attack = 1 + random(monster_info.level * 11) ;
      player_resist  = random(4 * player.dex) + armour_resist[player.armour] ;
      damage = 1 + random(monster_info.level) ;

      if (monster_attack > player_resist) {  //lint !e530
         // wsprintf(tempstr, 
         put_color_msg(TERM_MONSTER_HIT, _T("OUCH!!  It hit you!! [ %u points, A%u, D%u ]"), 
            damage, monster_attack, player_resist) ;
         // put_message(tempstr, attr_mon_hit); 

         if (damage > player.hit_points) {
            player.hit_points = 0 ;
            player_dies(hwnd) ;   //  this does not return
            return -1 ;
         }
         // player.hit_points -= monster_info.level ;
         player.hit_points -= damage ;
         show_hit_points() ;

         if (player.armour > 0) {
            apts = random(3);
            if (apts >= player.armour_points) {
               put_message(_T("Your armor has been destroyed.  Good Luck!!"));
               player.armour = 0; 
               player.armour_points = 0; 
            } else {
               player.armour_points -= apts ;
            }
            show_armour() ;
         }
      } else {
         _stprintf(tempstr, _T("What luck - he missed you... [ A%u, D%u ]"), 
            monster_attack, player_resist) ;
         put_message(tempstr); 
      } 
      break;

   case 1:
      put_message(_T("The web just broke!!  LOOK OUT!!")) ;
      web_turns-- ;
      break;

   default:
      _stprintf(tempstr, _T("The %s is stuck and can't attack now!"), monster_info.desc);
      put_message(tempstr) ;
      web_turns-- ;
      break;
   }
   return 1;   //  continue encounter
}

//*************************************************************
//  return:
//     0 when done with encounter
//     1 when encounter should continue
//    -1 if player is dead
//*************************************************************
static int attack_monster(HWND hwnd)
{
   if (player.weapon == 0) { 
      put_message(_T("The monster giggles as you pound it with your fists...")) ;
      put_message(_T("*Now* look what you did... it has the hiccups!!")) ;
      return monsters_turn(hwnd);   //  monster attacks at same time
   } 
   if (player.has_book) {
      put_message(_T("You can't beat it to death with a book!!")) ;
      return monsters_turn(hwnd);   //  monster attacks at same time
   } 
   //  note that this is independent of monster strength!!
   unsigned player_attack  = 5 * (1 + random(player.dex)) ;
   unsigned monster_resist = (6 * monster_info.level) 
                           + random(6 * monster_info.level) ;

   if (player_attack < monster_resist) {
      _stprintf(tempstr, _T("You missed, too bad... [ A%u, D%u ]"), player_attack, monster_resist); 
      put_message(tempstr); 
      return monsters_turn(hwnd);   //  monster attacks at same time
   } 
   unsigned player_damage = 1 + random(weapon_damage[player.weapon]) ;
   if (player_damage >= monster_info.hitpoints) {
      monster_info.hitpoints = 0 ;
      // wsprintf(tempstr, 
      put_color_msg(TERM_PLAYER_HIT, 
         _T("You hit the %s for %u points, killing it. [ A%u, D%u ]"), 
         monster_info.desc, player_damage, player_attack, monster_resist) ;
      // put_message(tempstr, attr_player_hit) ;
      return 0;
   } 
   monster_info.hitpoints -= player_damage ;
   // wsprintf(tempstr, 
   put_color_msg(TERM_PLAYER_HIT, 
      _T("You hit the %s. [ %u points, A%u, D%u, hp=%u ]"), 
      monster_info.desc, player_damage, player_attack, 
      monster_resist, monster_info.hitpoints) ;
   // put_message(tempstr, attr_player_hit) ;

   if (monster_info.index == GARGOYLE  ||  monster_info.index == DRAGON) {
      if (random(DIMEN_COUNT) == 4) { 
         put_message(_T("OH NO!!  Your weapon broke!!")) ;
         player.weapon=0;
         // show_status(hwnd) ;
         show_weapon() ;
      }
   }
   return monsters_turn(hwnd);   //  monster attacks at same time
}         

//*************************************************************
static int runaway_runaway(HWND hwnd)
{
   int result = monsters_turn(hwnd);      /* give monster one more swing !! */
   if (result >= 0) {
      draw_main_screen(NULL);
      show_player();
      put_message(_T("You have escaped !")) ;
      pop_keymap() ;
   }
   return result ;
}

//*************************************************************
//  return:
//     0 when done with encounter
//     1 when encounter should continue
//    -1 if player is dead
//*************************************************************
static int bribe_monster(HWND hwnd)
{
   unsigned contents, j ;

   if (player.treasure_count == 0) {
      put_message(_T("All I want from you is your life!!")) ;
      return monsters_turn(hwnd) ;
   } 

   //  pick a random treasure
   while (1) {
      j = random(DIMEN_COUNT) ;
      // tmask = 1 << j ;
      if (player.treasures[j]) {
         contents = TREASURE_BASE + j ;
         break;
      }
   }

   //  ask for it
   _stprintf(tempstr, _T("I'll take the %s..."), get_object_name(contents)) ;
   put_message(tempstr) ;
   monster_info.treasures[j] = 1 ;
   monster_info.treasure_count++ ;

   //  How do we store the fact that the monster now has the treasure??
   player.treasure_count-- ;
   // player.treasure_flags &= ~tmask ;
   player.treasures[j] = 0 ;

   if (monster_info.index == VENDOR) {
      vendor_angry = 0 ;
   }
   put_message(_T("OK, I'll let you off this time."));
   put_message(_T("Just watch your step!"));
   return 0;
}

//*************************************************************
int cast_spell(HWND hwnd, unsigned inchr)
{
   int result = 1 ;  //  by default, continue rounds after this
   unsigned iTemp ;
   
   switch (inchr) {
   case kw:
      if (player.iq < 10) {
         put_message(_T("You're too dumb to cast a Web spell")) ;
         monsters_turn(hwnd) ;
         break;
      }
      player.iq-- ;
      show_int() ;

      web_turns = random(9); 
      put_message(_T("You wave your arm quickly...")) ;
      _stprintf(tempstr, _T("The %s is firmly encased in a strong web"), 
            monster_info.desc) ;
      put_message(tempstr) ;
      break;

   case kf:
      if (player.iq < 12) {
         put_message(_T("You're too dumb to cast a Fireball spell")) ;
         monsters_turn(hwnd) ;
         break;
      }
      player.iq-- ;
      if (player.dex <= 1) {
         put_message(_T("You fumble the spell and destroy yourself!!")) ;
         player.dex = 0 ;
         player_dies(hwnd) ;
         return -1;
      }
      player.dex-- ;
      show_int() ;
      show_dex() ;

      put_message(_T("You wave your arm, and a glowing ball flies from it."));
      iTemp = (1 + random(monster_info.hitpoints)) ;
      if (iTemp < 5)
          iTemp = 5 ;
      if (iTemp >= monster_info.hitpoints) {
         monster_info.hitpoints = 0 ;
         _stprintf(tempstr, _T("The %s is annihilated by the blast."), monster_info.desc) ;
         put_message(tempstr) ;
         put_message(_T("You are covered with charred flesh."));
         break;
      }
      monster_info.hitpoints -= iTemp ;
      _stprintf(tempstr, _T("The %s is injured by the blast [ hp=%u ]."), 
         monster_info.desc, monster_info.hitpoints) ;
      put_message(tempstr) ;
      put_message(_T("You are covered with charred  flesh."));
      result = monsters_turn(hwnd) ;
      break;

   case kd:
      if (player.iq < 15) {
         put_message(_T("The Deathspell backfires!!!")) ;
         player.iq = 0 ;
         player_dies(hwnd) ;
         return -1;
         // result = -1 ;
         // break;
      }
      if ((unsigned) player.iq < monster_info.hitpoints) {
         _stprintf(tempstr, _T("The %s's magic was stronger than yours..."), 
               monster_info.desc) ;
         put_message(tempstr) ;
         // delay(1200) ;
         player.iq = 0 ;
         player_dies(hwnd) ;
         return -1;
         // result = -1 ;
         // break;
      }
      monster_info.hitpoints = 0;
      put_message(_T(" ")) ;
      _stprintf(tempstr, _T("The %s glows brightly, "), monster_info.desc) ;
      put_message(tempstr) ;
      put_message(_T("then disappears before your eyes!"));
      put_message(_T(" ")) ;
      break;

   default:
      put_message(_T("PEONS CAN'T USE THAT SPELL!!")) ;
      result = monsters_turn(hwnd) ;
      break;
   }
   
   pop_keymap() ; //  pop back to KEYMAP_MONSTER
   if (monster_info.hitpoints == 0) {
      EndEncounter() ;  //  cast_spell()
      pop_keymap() ; //  pop back to KEYMAP_DEFAULT
      result = 0;
   }
   return result ;
}

//*************************************************************
static unsigned combat_round ;

static void show_combat_info(void)
{
   // sprintf(tempstr, "YOU: str=%2d, int=%d, dex=%d, armour=%d", 
   //    player.str, player.iq, player.dex, player.armour_points) ;
   // put_message(tempstr) ;
   infoout(_T("round %u: You're facing %s %s [ L%u, hp=%u ]"), 
      combat_round++, 
      // starts_with_vowel(monster_info.desc) ? _T("an") : _T("a"),
      get_monster_prefix(monster_info.desc), 
      monster_info.desc, monster_info.level, monster_info.hitpoints) ;
   // put_message(tempstr, attr_new_round) ;
   put_message(_T("You may: [A]ttack, [R]etreat, [C]ast a spell.")) ;
   if (player.treasure_count > 0) {
      put_message(_T("You may also attempt a bribe.")) ;
   }
   put_message(_T("Your choice?? ")) ;
}

//*************************************************************
//  return:
//     0 when done with encounter
//     1 when encounter should continue
//    -1 if player is dead
//*************************************************************
int run_one_encounter_round(HWND hwnd, unsigned inchr)
{
   int result ;
   switch (inchr) {
   case ka:  
      term_append(_T("Attack")) ;
      result = attack_monster(hwnd) ;  
      break ;

   case kr:  
      term_append(_T("Run For My Life !!")) ;
      result = runaway_runaway(hwnd) ;  
      if (result >= 0) {
         move_one_square(hwnd) ;
         goto quiet_exit;
      }
      break ;

   case kb:  
      term_append(_T("Bribe")) ;
      result = bribe_monster(hwnd) ;  
      if (result == 0) 
         goto quiet_exit;
      break ;

   //  This one is tricky, because the remainder of spell
   //  casting won't be handled here, it will be in the
   //  KEYMAP_SPELL* states. 
   //  Normally, spell-casting will pop_keymap when done,
   //  but if the monster is killed, they need to pop
   //  back to the KEYMAP_DEFAULT state.
   case kc:  
      term_append(_T("Cast Spell")) ;
      put_message(_T("Which spell? <Web, Fireball, Deathspell> ")) ;
      push_keymap(KEYMAP_SPELL_SELECTION) ;
      result = 1 ;
      goto quiet_exit;

   default:
      term_append(_T("odd gesture [%u]"), inchr) ;
      _stprintf(tempstr, _T("The %s is not impressed..."), monster_info.desc);
      put_message(tempstr);
      result = monsters_turn(hwnd) ;
      if (result >= 0) {
         show_combat_info() ;
      }
   }  //  switch

   if (result == 0) {
      EndEncounter() ;
   }
   else if (result > 0) {
      show_combat_info() ;
   }

quiet_exit:
   return result ;
}

//*************************************************************
//  this enters combat mode
//*************************************************************
void react_to_monsters(HDC hdcUnused)
{
   // ZeroMemory((char *) &monster_info, sizeof(monster_info_t)) ;
   monster_info.index     = get_room_contents() ;
   monster_info.desc      = get_room_contents_str() ;
   monster_info.level     = monster_info.index - MONSTER_BASE + 1;   //  base:1
   monster_info.hitpoints = monster_info.level + random(2 * monster_info.level) + 1 ;
   combat_round = 0 ;

   // pngCombat.render_bitmap(hdc, 0, 0);
   render_combat_bitmap() ;
   show_combat_info() ;
   push_keymap(KEYMAP_MONSTER) ;
}

