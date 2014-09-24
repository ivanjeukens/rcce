CAD	= /home/ivan/oldstuff/RCCE
SIS	= /home/ivan/oldstuff/RCCE/sis

P	= arch
PSRC	= archGraph.c archParse.c archBlock.c archAbGen.c
POBJ	= $(PSRC:.c=.o)
PHDR	= arch.h archInt.h
MISC	= Makefile

TARGET	= libarch.so
HDR	=

LIBS	= 
INCLUDE	= -I$(CAD)/include -I$(SIS)/include

CFLAGS	= $(INCLUDE) -Wall
LDFLAGS	= -shared

#---------------------------------------------------------------------------

$(TARGET): $(PHDR) $(OBJ) $(POBJ) $(LIBS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJ) $(POBJ) $(LIBS)
	
lib$(P).a : $(POBJ)
	rm -f lib$(P).a
	ar cr lib$(P).a $(POBJ)
	ranlib lib$(P).a

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
