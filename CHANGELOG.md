# WinWiz Changelog

## [1.47] - 2025-06-13
- update player description after character changes
- fix stat-button enables and treasure handling in Vendor dialog

## [1.46] - 2025-05-04
- bug (fixed): after telePort to monster location, keymap was wrong.

## [1.45] - 2025-05-02
- Fix bugs, code cleanup
- added debug command and functions
- fixed bug: incorrectly drawn frames for empty room after moving through them.
- add caching of sprite resources at startup, to speed up map redraws.

## [1.44] - 2025-04-29
- Converting to Unicode, to support gdiplus library
- Completed gdiplus conversion, eliminating loadpng library

## [1.43] - 2025-04-29
- Executing Chest did not remove the chest afterwards, in some cases.

## [1.42] - 2023-01-06
- Add message when old commands are used; update help file

## [1.41] - 2023-10-26
- Add the [E]xecute command, which replaces [O]pen, [G]aze, d[R]ink, and [T]rade

## [1.40] - 2021-06-07
- Fix bug where, when player dies while pressing invalid keys, invalid text is displayed
- Update the .chm help file, for first time since 2006

## [1.39] - 2021-06-06
- add display of damage done after chest explosion
- fix bug after teleport, keymap in wrong state and keyboard is handled wrong

## [1.38] - 2014-04-22
- Modify LodePng class to allow simpler calls to sprite-rendering function.

## [1.37] - 2014-04-18
- fix bug in handling position awareness
- added tooltips to treasure fields
- speed up main-loop (cursor) timer, so restore and other redraws occur more promptly.

## [1.36] - 2014-04-16
- Revising handling of main-screen controls

## [1.35] - 2014-04-14
- Add new function to LodePng class, which handles image lists by accepting an image index in rendering function, and calculating offsets into image list from that index.
- add static frames around the treasure icons

## [1.34] - 2014-04-13
- at startup, place program in center of screen
- fix death message so that it identifies creature carrying Runestaff
- implement obscured location (i.e., don't show position and level) after hitting Sinkhole or Warp, until a known level is reached.

## [1.33] - 2014-04-11
- Convert from tiles32.bmp to tiles32.png
- merge PNG interface into a separate class

## [1.32] - 2012-08-14
- Incorporate current common libs and classes

## [1.31] - 2011-01-05
- Add GroupBox around new Treasures block
- Remove Treasures field from Player Info GroupBox

## [1.30] - 2011-01-05
- Converting to Dialog vs Window interface, using terminal module.

## [1.22] - 2008-08-08
- Switch back to .BMP files, which look much better.
- Lint the code with PcLint V8.00x

## [1.20] - 2007-06-20
- Fixing some bugs in handling of new status windows.
- Modify odds of various "Open Chest" options, to make good alternatives more likely.
- Fix a bug when stumbling from room

## [1.19] - 2007-06-19
- Re-design status windows so they use actual windows and static controls, rather than raw dprints writes.

## [1.17] - 2006-12-07
- jpeg read/draw functionality is now in a class, and supports tile-painting as needed for the sprites.

## [1.16] - 2006-12-05
- Add module to support reading jpeg files. So far, though, this doesn't work with the tiles file, because I need to preserve the structs.

## [1.15] - 2006-04-11
- Fix help-file comments

## [1.14] - 2005-09-21
- vendor: Clean up handling of armour/weapon selection. (hmmm... closer, but not done yet...)

## [1.13] - 2005-09-20
- Fix bug: one could buy equipment they couldn't afford!

## [1.12] - 2005-09-20
- Linted code with new lint definition file
- Made Gaze more useful in early game

## [1.11] - 2005-09-19
- Add INT option to book opening

## [1.10] - 2005-09-18
- Add some color at death, to hint at where the runestaff and orb were.

## [1.09] - 2005-09-05
- Use intelligent method to find help file

## [1.08] - 2005-01-03
- Don't pop keymap when game is won
- Modify monster-description messages

## [1.07] - 2005-01-02
- attribute damage now starts at 1, not 0
- fix bugs in handling telePort

## [1.06] - 2005-01-02
- Fixed bug in Vendor/Dex incrementing

## [1.05] - 2004-12-26
- Fixed bug in Curse of the Leech bug fix (duh...)

## [1.04] - 2004-12-25
- Fixed bug in Curse of the Leech

## [1.03] - 2004-12-23
- Add support for INI files for colors
- Fix bug in curing Curse of Amnesia

## [1.02] - 2004-12-23
- Fixed numerous bugs

## [1.01] - 2004-12-15
- original release Win32/C release
