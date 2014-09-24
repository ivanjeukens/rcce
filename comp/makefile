CAD	= /home/i/RCCE
SIS	= /home/i/RCCE/sis

P	= comp
PSRC	= compUtil.c compLg.c compRg.c compParse.c
POBJ	= $(PSRC:.c=.o)
PHDR	= comp.h compInt.h
MISC	= Makefile

TARGET	= libcomp.so
HDR	=

LIBS	= 
INCLUDE	= -I$(CAD)/include -I$(SIS)/include

CFLAGS	= $(INCLUDE) -Wall
LDFLAGSD = -gdwarf3 -shared
LDFLAGSS = -g
LINTFLAGS = $(INCLUDE) +gnuextensions

#---------------------------------------------------------------------------

$(TARGET): $(PHDR) $(OBJ) $(POBJ) $(LIBS)
	$(CC) $(LDFLAGSD) -o $(TARGET) $(OBJ) $(POBJ) $(LIBS)
	
lib$(P).a : $(POBJ)
	rm -f lib$(P).a
	ar cr lib$(P).a $(POBJ)
	ranlib lib$(P).a
	
install: lib$(P).so llib-l$(P).ln

lint: $(PSRC) $(PHDR) $(SRC) $(HDR)
	lclint $(LINTFLAGS) $(SRC) $(PSRC)

clean:
	rm -f $(TARGET) *.so *.ln *.o *.a \
	    [Tt]ags [Mm]ake.out lint malloc.out gmon.out __.SYMDEF

depend:
	sed '/^#--DO NOT CHANGE ANYTHING AFTER THIS LINE/,$$d' Makefile >mktemp
	echo '#--DO NOT CHANGE ANYTHING AFTER THIS LINE' >>mktemp
	$(CAD)/bin/cc-M $(CFLAGS) $(PSRC) | \
	    sed 's|$(CAD)|$$(CAD)|g' | \
	    grep -v '/usr/include' >>mktemp
	mv mktemp Makefile

strip_depend:
	sed '/^#--DO NOT CHANGE ANYTHING AFTER THIS LINE/,$$d' Makefile >mktemp
	mv mktemp Makefile
_force:
