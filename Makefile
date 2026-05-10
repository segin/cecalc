TOOLCHAIN_PREFIX = /opt/arm-mingw32ce/bin/arm-mingw32ce-
CC = $(TOOLCHAIN_PREFIX)gcc
CXX = $(TOOLCHAIN_PREFIX)g++
STRIP = $(TOOLCHAIN_PREFIX)strip
WINDRES = $(TOOLCHAIN_PREFIX)windres

COMMON_FLAGS = -O2 -Wall -Wno-unused-parameter -MMD -MP \
               -D_WIN32_WCE=0x600 -D_UNICODE -DUNICODE \
               -DNO_HEAP_HOOKS \
               -Isrc -Isrc/ratpak

CFLAGS   = $(COMMON_FLAGS) -std=gnu99
CXXFLAGS = $(COMMON_FLAGS) -fpermissive -fexceptions -Wno-write-strings \
           -fkeep-inline-functions
RATPAK_CXXFLAGS = $(CXXFLAGS) -Wno-unknown-pragmas -Wno-unused-value \
                  -Wno-unused-variable -Wno-unused-but-set-variable \
                  -Wno-parentheses -Wno-switch
RATPAK_CFLAGS = $(CFLAGS) -Wno-missing-braces -Wno-override-init
RATPAK_SUPPORT_CFLAGS = $(RATPAK_CFLAGS) -w

LDFLAGS  = -lcommctrl -lcommdlg -lcoredll -static-libgcc -static-libstdc++

CALC_SRC = src/calc.cpp src/input.cpp src/scicomm.cpp src/scidisp.cpp \
           src/scifunc.cpp src/scikeys.cpp src/scimath.cpp src/scimenu.cpp \
           src/scioper.cpp src/sciproc.cpp src/sciset.cpp src/scistat.cpp \
           src/unifunc.cpp src/wassert.cpp src/ce_compat.cpp

RATPAK_SRC = src/ratpak/basex.c src/ratpak/conv.c src/ratpak/debug.c \
             src/ratpak/exp.c src/ratpak/fact.c src/ratpak/itrans.c \
             src/ratpak/itransh.c src/ratpak/logic.c src/ratpak/num.c \
             src/ratpak/rat.c src/ratpak/support.c src/ratpak/trans.c \
             src/ratpak/transh.c

CALC_OBJ   = $(CALC_SRC:.cpp=.o)
RATPAK_OBJ = $(RATPAK_SRC:.c=.o)
OBJ        = $(CALC_OBJ) $(RATPAK_OBJ)
DEP        = $(OBJ:.o=.d)
RES        = src/resources.o
TARGET     = cecalc.exe

all: $(TARGET)

$(TARGET): $(OBJ) $(RES)
	$(CXX) $(OBJ) $(RES) -o $@ $(LDFLAGS)
	$(STRIP) $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# support.c includes ratconst.h, whose static NUMBER initializers rely on
# zero-length array initialization (a C-only Microsoft extension). Build that
# one file as C; the throw-using ratpak files still go through g++.
src/ratpak/support.o: src/ratpak/support.c
	$(CC) $(RATPAK_SUPPORT_CFLAGS) -c $< -o $@

src/ratpak/%.o: src/ratpak/%.c
	$(CXX) $(RATPAK_CXXFLAGS) -x c++ -c $< -o $@

src/resources.o: src/resources.rc src/resource.h src/calc.ico
	$(WINDRES) -Isrc $< $@

clean:
	rm -f $(OBJ) $(DEP) $(RES) $(TARGET)

-include $(DEP)
