### WinWiz - a win32 port of the ancient DOS game Wizard's Castle

MANY CYCLES AGO, IN THE KINGDOM OF N'DIC, THE GNOMIC
WIZARD ZOT FORGED HIS GREAT *ORB OF POWER*.  HE SOON
VANISHED, LEAVING BEHIND HIS VAST SUBTERRANEAN CASTLE
FILLED WITH ESURIENT MONSTERS, FABULOUS TREASURES,
AND THE INCREDIBLE *ORB OF ZOT*.  FROM THAT TIME HENCE,
MANY A BOLD YOUTH HAS VENTURED INTO THE WIZARD'S CASTLE.
AS OF NOW, *NONE* HAS EVER EMERGED VICTORIOUSLY!  BEWARE!!

<hr>

#### building the application
This application is built using the MinGW toolchain; 
I recommend the [TDM](http://tdm-gcc.tdragon.net/) distribution, 
to avoid certain issues with library accessibility. 
The makefile also requires certain Cygwin tools (rm, make, etc).

#### NOTE: this program requires my ```der_libs``` submodule
If you clone the repository without the --recursive flag, 
you can recover the submodule later, with this command:

```git submodule update --init --recursive```

<hr>
Beginning in April 2025, the LodePng library is now deprecated,
and UNICODE support is now required, for the GDI+ library that has replaced it.

<hr>
This project is licensed under Creative Commons CC0 1.0 Universal;  
https://creativecommons.org/publicdomain/zero/1.0/

The person who associated a work with this deed has dedicated the work to the
public domain by waiving all of his or her rights to the work worldwide under
copyright law, including all related and neighboring rights, to the extent
allowed by law.

You can copy, modify, distribute and perform the work, even for commercial
purposes, all without asking permission. 

