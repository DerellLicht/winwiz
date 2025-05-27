#*************************************************************************************
# NOTE regarding UNICODE support
# This application now relies on GDI+ library, which in turn requires UINCODE.
# Thus, the non-Unicode build is no longer supported.
# That build required LodePng library, which provided support for .png files,
# but no other extensions, and is now deprecated for all of my graphics programs.
#*************************************************************************************
USE_DEBUG = NO
USE_UNICODE = YES
USE_64BIT = NO

ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
#TOOLS=c:\mingw\bin
#TOOLS=c:\TDM-GCC-64\bin
TOOLS=c:\tdm32\bin
endif

#*****************************************************************************
# notes on compiler quirks, using MinGW/G++ V4.3.3
# if I build with -O3, I get following warnings from g++ :
#   wfuncs.cpp: In function 'int light_a_flare(HWND__*)':
#   wfuncs.cpp:338: warning: array subscript is above array bounds
# where light_a_flare() starts at line 779 !!
# If I build with -O2, I get no such warnings.
# In either case, PcLint V9 is giving no warnings on this code.
#*****************************************************************************

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g -mwindows 
LFLAGS=
else
CFLAGS=-Wall -O2 -mwindows -Weffc++ 
LFLAGS=-s
endif
CFLAGS += -Wno-write-strings

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
LFLAGS += -dUNICODE -d_UNICODE
LiFLAGS += -DUNICODE -D_UNICODE
else
CFLAGS += -Wno-stringop-truncation
CFLAGS += -Wno-conversion-null
endif

# link library files
LiFLAGS += -Ider_libs
CFLAGS += -Ider_libs
CSRC=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/winmsgs.cpp \
der_libs/statbar.cpp \
der_libs/cterminal.cpp \
der_libs/terminal.cpp \
der_libs/tooltips.cpp \
der_libs/vlistview.cpp 

# separate local source files from library files,
# so that wc operation is more appropriate.
CBASE=winwiz.cpp globals.cpp keyboard.cpp wfuncs.cpp \
CastleInit.cpp initscrn.cpp combat.cpp vendor.cpp loadhelp.cpp \
about.cpp hyperlinks.cpp
CSRC += $(CBASE)

#  clang-tidy options
CHFLAGS = -header-filter=.*
CHTAIL = -- -Ider_libs
ifeq ($(USE_64BIT),YES)
CHTAIL += -DUSE_64BIT
endif
ifeq ($(USE_UNICODE),YES)
CHTAIL += -DUNICODE -D_UNICODE
endif

OBJS = $(CSRC:.cpp=.o) rc.o

BASE=winwiz
BIN=$(BASE).exe

LIBS= -lgdi32 -lcomctl32 -lhtmlhelp -lolepro32 -lole32 -luuid

# none of the BMP/JPG code is relevant, if UNICODE is defined
ifeq ($(USE_UNICODE), YES)
CSRC+=der_libs/gdi_plus.cpp der_libs/gdiplus_setup.cpp
LIBS += -lgdiplus 
IMAGES=tiles32.png images.png
endif

#************************************************************
%.o: %.cpp
	$(TOOLS)\g++ $(CFLAGS) -c $< -o $@

#************************************************************
all: $(BIN)

clean:
	rm -vf $(BIN) $(OBJS) *.zip *.bak *~

check:
	cmd /C "d:\clang\bin\clang-tidy.exe $(CHFLAGS) $(CSRC) $(CHTAIL)"

dist:
	rm -f $(BASE).zip
	zip $(BASE).zip *.exe winwiz.chm $(IMAGES) history.winwiz.txt LICENSE readme.md

wc:
	wc -l $(CBASE) *.rc

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CSRC)"

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************
winwiz.exe: $(OBJS)
	$(TOOLS)\g++ $(CFLAGS) $(LFLAGS) $(OBJS) -o $@ $(LIBS)

rc.o: winwiz.rc 
	$(TOOLS)\windres $< -O coff -o $@

# DO NOT DELETE

der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
der_libs/cterminal.o: der_libs/common.h der_libs/commonw.h
der_libs/cterminal.o: der_libs/cterminal.h der_libs/vlistview.h
der_libs/terminal.o: der_libs/common.h der_libs/commonw.h
der_libs/terminal.o: der_libs/cterminal.h der_libs/vlistview.h
der_libs/terminal.o: der_libs/terminal.h der_libs/winmsgs.h
der_libs/tooltips.o: der_libs/iface_32_64.h der_libs/common.h
der_libs/tooltips.o: der_libs/tooltips.h
der_libs/vlistview.o: der_libs/common.h der_libs/commonw.h
der_libs/vlistview.o: der_libs/vlistview.h
winwiz.o: resource.h version.h der_libs/common.h der_libs/commonw.h
winwiz.o: der_libs/statbar.h der_libs/cterminal.h der_libs/vlistview.h
winwiz.o: der_libs/terminal.h der_libs/winmsgs.h wizard.h keywin32.h
winwiz.o: der_libs/tooltips.h der_libs/gdiplus_setup.h
globals.o: der_libs/common.h wizard.h
keyboard.o: der_libs/common.h wizard.h keywin32.h
wfuncs.o: resource.h der_libs/common.h der_libs/commonw.h wizard.h keywin32.h
wfuncs.o: der_libs/terminal.h der_libs/gdi_plus.h
CastleInit.o: der_libs/common.h wizard.h
initscrn.o: resource.h der_libs/common.h wizard.h
combat.o: der_libs/common.h wizard.h keywin32.h
vendor.o: resource.h der_libs/common.h wizard.h
loadhelp.o: der_libs/common.h
about.o: resource.h version.h der_libs/common.h wizard.h hyperlinks.h
hyperlinks.o: der_libs/iface_32_64.h hyperlinks.h
der_libs/gdi_plus.o: der_libs/common.h der_libs/gdi_plus.h
der_libs/gdiplus_setup.o: der_libs/gdi_plus.h
