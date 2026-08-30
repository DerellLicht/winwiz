#*************************************************************************************
# NOTE regarding UNICODE support
# This application now relies on GDI+ library, which in turn requires UINCODE.
# Thus, the non-Unicode build is no longer supported.
#*************************************************************************************
USE_DEBUG = NO
USE_UNICODE = YES
USE_64BIT = NO
USE_CLANG = NO
# sadly, cygwin mingw does not support gdiplus...
USE_CYGWIN = NO

include der_libs\tool_select.mak

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g -mwindows 
LFLAGS=
else
CFLAGS=-Wall -O2 -Weffc++ -c 
LFLAGS=-s
endif
CFLAGS += -Wno-write-strings

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
LFLAGS += -dUNICODE -d_UNICODE
LiFLAGS += -DUNICODE -D_UNICODE
IFLAGS += -DUNICODE -D_UNICODE
else
CFLAGS += -Wno-stringop-truncation
CFLAGS += -Wno-conversion-null
endif

ifeq ($(USE_CLANG),YES)
CFLAGS += -DUSING_CLANG
endif
LFLAGS += -mwindows 

ifeq ($(USE_STATIC),YES)
LFLAGS += -static
endif

IFLAGS += -Ider_libs

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
der_libs/hyperlinks.cpp \
der_libs/vlistview.cpp 

# separate local source files from library files,
# so that wc operation is more appropriate.
CBASE=winwiz.cpp keyboard.cpp wfuncs.cpp about.cpp \
CastleInit.cpp initscrn.cpp combat.cpp vendor.cpp loadhelp.cpp 

CSRC += $(CBASE)

LINTFILES=lintdefs.cpp lintdefs.ref.h 

OBJS = $(CSRC:.cpp=.o) rc.o

BASE=winwiz
BIN=$(BASE).exe

LIBS= -lgdi32 -lcomdlg32 -lhtmlhelp -lolepro32 -lole32 -luuid

# none of the BMP/JPG code is relevant, if UNICODE is defined
ifeq ($(USE_UNICODE), YES)
CSRC+=der_libs/gdi_plus.cpp der_libs/gdiplus_setup.cpp
LIBS += -lgdiplus 
IMAGES=tiles32.png images.png
endif

# Automatically parse the latest version block
VERSION := $(shell grep -oE '\[[0-9]+\.[0-9]+\]' CHANGELOG.md | head -n 1 | tr -d '[]')
DIST_ZIP := $(BASE)V$(VERSION).zip

.PHONY: dist release update
#************************************************************
%.o: %.cpp
	$(TOOLS)\$(GNAME) $(CFLAGS) $< -o $@

#************************************************************
all: $(BIN)

clean:
	rm -vf $(BIN) $(OBJS) *.zip *.bak *~

dist:
	rm -f $(BASE).zip
	zip $(DIST_ZIP) *.exe $(BASE).chm $(IMAGES) LICENSE.txt readme.md CHANGELOG.md

# Your new automated release workflow
release: dist
	cmd /C "@echo Preparing GitHub release for v$(VERSION)..."
	sed -n '/## \['$(VERSION)'\]/,/## \[/p' CHANGELOG.md | sed '$$d' > temp_notes.md
	gh release create v$(VERSION) ./$(DIST_ZIP) ./CHANGELOG.md --notes-file temp_notes.md
	rm temp_notes.md
	cmd /C "@echo Release v$(VERSION) successfully uploaded to GitHub!"
	
# Your new update-in-place pipeline
update: dist
	cmd /C "@echo Updating assets for existing release v$(VERSION)..."
	@# Uploads and overwrites the .zip file and CHANGELOG.md on GitHub
	gh release upload v$(VERSION) ./$(DIST_ZIP) ./CHANGELOG.md --clobber
	cmd /C "@echo Release v$(VERSION) assets successfully updated on GitHub!"

wc:
	wc -l $(CBASE) *.rc

cppc:
	cmd /C "cppcheck --project=compile_commands.json --check-level=exhaustive --enable=all --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CSRC)"

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"
	
lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) $(LINTFILES) $(CSRC)"

depend:
	makedepend $(IFLAGS) $(CSRC)

#************************************************************
winwiz.exe: $(OBJS)
	$(TOOLS)/g++ $(LFLAGS) $(OBJS) -o $@ $(LIBS)

# note: though all other utilities can accept forward slash in paths,
#       windres cannot... 
rc.o: winwiz.rc 
	$(TOOLS)\$(WRNAME) $< -O COFF -o $@

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
der_libs/hyperlinks.o: der_libs/iface_32_64.h der_libs/hyperlinks.h
der_libs/vlistview.o: der_libs/common.h der_libs/commonw.h
der_libs/vlistview.o: der_libs/vlistview.h
winwiz.o: resource.h version.h der_libs/common.h der_libs/commonw.h
winwiz.o: der_libs/statbar.h der_libs/cterminal.h der_libs/vlistview.h
winwiz.o: der_libs/terminal.h der_libs/winmsgs.h wizard.h keywin32.h
winwiz.o: der_libs/tooltips.h der_libs/gdiplus_setup.h
keyboard.o: der_libs/common.h wizard.h keywin32.h
wfuncs.o: resource.h der_libs/common.h der_libs/commonw.h wizard.h keywin32.h
wfuncs.o: der_libs/terminal.h der_libs/gdi_plus.h
about.o: resource.h version.h der_libs/common.h wizard.h
about.o: der_libs/hyperlinks.h
CastleInit.o: der_libs/common.h wizard.h
initscrn.o: resource.h der_libs/common.h der_libs/commonw.h wizard.h
combat.o: der_libs/common.h wizard.h keywin32.h
vendor.o: resource.h der_libs/common.h wizard.h
loadhelp.o: der_libs/common.h
der_libs/gdi_plus.o: der_libs/common.h der_libs/gdi_plus.h
der_libs/gdiplus_setup.o: der_libs/gdi_plus.h
