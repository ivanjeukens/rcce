CAD	= /home/i/RCCE
SIS	= /home/i/RCCE/sis

P	= intf
PSRC	= intfMain.c intfCommand.c intfUtils.c intfBatch.c intfArch.c \
	  intfAb.c intfComp.c
POBJ	= $(PSRC:.c=.o)
PHDR	= intf.h intfInt.h
MISC	= Makefile

TARGET	= libintf.so
HDR	=

LIBS	= 
INCLUDE	= -I$(CAD)/include -I$(SIS)/include

CFLAGS	= $(INCLUDE) -Wall
LDFLAGD	= -gdwarf3 -shared
LDFLAGS = -g

#---------------------------------------------------------------------------

$(TARGET): $(PHDR) $(OBJ) $(POBJ) $(LIBS)
	$(CC) $(LDFLAGD) -o $(TARGET) $(OBJ) $(POBJ) $(LIBS)
#	$(CC) -gdwarf3 $(INCLUDE) -c main.c
	
install: lib$(P).so llib-l$(P).ln

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
