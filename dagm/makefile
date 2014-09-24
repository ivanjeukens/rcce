CAD	= /home/i/RCCE
SIS	= /home/i/RCCE/sis

P	= dagm
PSRC	= dagmCom.c dagmInt.c
POBJ	= $(PSRC:.c=.o)
PHDR	= dagm.h dagmInt.h
MISC	= Makefile

TARGET	= libdagm.so
HDR	=

LIBS	= 
INCLUDE	= -I$(CAD)/include -I$(SIS)/include

CFLAGS	= $(INCLUDE) -Wall
LDFLAGSD = -gdwarf3 -shared
LDFLAGSS = -g

#---------------------------------------------------------------------------

$(TARGET): $(PHDR) $(OBJ) $(POBJ) $(LIBS)
	$(CC) $(LDFLAGSD) -o $(TARGET) $(OBJ) $(POBJ) $(LIBS)
	
lib$(P).a : $(POBJ)
	rm -f lib$(P).a
	ar cr lib$(P).a $(POBJ)
	ranlib lib$(P).a
	
install: lib$(P).so llib-l$(P).ln

clean:
	rm -f $(TARGET) *.so *.ln *.o \
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
