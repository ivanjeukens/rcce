CAD	= /home/ivan/oldstuff/RCCE
CADROOT	= /home/ivan/oldstuff
SIS	= /home/ivan/oldstuff/RCCE/sis
UNPACK	= unpack
COPY = .
SHELL = /bin/sh
MAKE = /usr/bin/make
REQUIRE = 

#DIRS = comp arch intf llib util2 netl draw dmig dagm \
#      levm ola place rou
DIRS = 

DIRD = comp arch intf llib util2 netl draw dmig dagm \
       levm place rou
#DIRD = 

TARGET	= pro

TARLIB	= libpro.so

#LIBSS	= -lm -lX11 -lxview -lolgx -Wl,-Bstatic -lintf -larch -lcomp \
#	  -ldraw -lllib -lnetl -ldmig -ldagm -llevm -lutil2 -lplace \
#	  -Wl,-Bdynamic \
#	  -larray -lastg -latpg -lavl -lclock -ldecomp -ldelay -lenc \
#	  -lerror -lespresso -lextract -lfactor -lgenlib -lgraph -lio \
#	  -llatch -llist -lmap -lmaxflow -lmincov -lminimize \
#	  -lnetwork -lnode -lntbdd -lorder -lphase -lresub -lseqbdd \
#	  -lsimplify -lsparse -lspeed -lst -lstg -lutil -lvar_set -lcommand \
#	  -lbdd -lmtr
LIBSS =

#LIBSD =
LIBSD	= -lm -lX11 -lxview -lolgx -Wl,-Bdynamic \
	  -larray -lastg -latpg -lavl -lclock -ldecomp -ldelay -lenc \
	  -lerror -lespresso -lextract -lfactor -lgenlib -lgraph -lio \
	  -llatch -llist -lmap -lmaxflow -lmincov -lminimize \
	  -lnetwork -lnode -lntbdd -lorder -lphase -lresub -lseqbdd \
	  -lsimplify -lsparse -lspeed -lst -lstg -lutil -lvar_set -lcommand \
	  -lbdd -lmtr -lpld \
	  -lintf -larch -lcomp -ldraw -lutil2 -lllib -lnetl -ldmig -ldagm \
	  -llevm -lplace -lrou

	  
CC	= gcc
CFLAGSD	= -fPIC -DSIS -DDEBUG -Wall
CFLAGSS = -g -DSIS -DDEBUG -Wall

LDFLAGS = -L$(SIS)/lib -L$(CAD)/lib -L/usr/X11/lib \
	  -L/usr/openwin/lib

#---------------------------------------------------------------------------

$(TARGET): $(TARLIB) do_link

$(TARLIB): do_make do_lns
#	cp main/sis.o lib/sis.o
$(TARLIBG): do_touch do_makeg do_lns
	mv $(TARLIB) $(TARLIBG)
$(TARLINT): do_lintlib do_repack_lint

do_link:
	$(CC) -o $(TARGET) $(LDFLAGS) $(LIBSD) $(LIBSS) 

do_make:
	@for dir in $(DIRD); do			\
	    (cd $$dir; 					\
	     echo Making Dynamic $$dir ...;		\
	     ${MAKE} CC=$(CC) 'CFLAGS=$(CFLAGSD) $$(INCLUDE)' SIS=$(SIS) CADROOT=$(CADROOT) CAD=$(CAD) lib$$dir.so)\
	done
	@for dir in $(DIRS); do			\
	    (cd $$dir; 					\
	     echo Making Static $$dir ...;		\
	     ${MAKE} CC=$(CC) 'CFLAGS=$(CFLAGSS) $$(INCLUDE)' SIS=$(SIS) CADROOT=$(CADROOT) CAD=$(CAD)	lib$$dir.a)\
	done
	
do_lns:
	@for dir in $(DIRD); do	   \
	   (echo Making library link $$dir ...; \
	   ln -f -s $(CAD)/$$dir/lib$$dir.so $(CAD)/lib/lib$$dir.so) \
	done
	@for dir in $(DIRS); do	   \
	   (echo Making library link $$dir ...; \
	   ln -f -s $(CAD)/$$dir/lib$$dir.a $(CAD)/lib/lib$$dir.a) \
	done

clean: cleansome

cleansome:
	rm -f lib/*
	@for dir in $(DIRS); do				\
	    (cd $$dir; 					\
	     echo Cleaning $$dir ...; 			\
	     ${MAKE} -i CAD=$(CAD) SIS=$(SIS) strip_depend >/dev/null	\
	     ${MAKE} -i CAD=$(CAD) SIS=$(SIS) clean >/dev/null)	\
	done
	@for dir in $(DIRD); do				\
	    (cd $$dir; 					\
	     echo Cleaning $$dir ...; 			\
	     ${MAKE} -i CAD=$(CAD) SIS=$(SIS) strip_depend >/dev/null	\
	     ${MAKE} -i CAD=$(CAD) SIS=$(SIS) clean >/dev/null)	\
	done

backup:
	@rm -f rcce.tar
	@tar -cf rcce.tar Makefile
	@for dir in $(DIRS); do			\
	  (tar -rf rcce.tar ./$$dir/*.c ./$$dir/*.h ./$$dir/Makefile)	\
        done					
	@for dir in $(DIRD); do 		\
          (tar -rf rcce.tar ./$$dir/*.c ./$$dir/*.h ./$$dir/Makefile)	\
	done
	@gzip -9 rcce.tar

	
wc:
	wc */*.[cly] | tail -1
	grep ';' */*.[cly] | wc | tail -1

compile:
	rm -f io/read_eqn.c io/eqnlex.c 
	rm -f genlib/readlib.c genlib/readliblex.c
	date; ${MAKE} $(TARGETG)
	date; ${MAKE} cleansome $(TARGET) 
	date; ${MAKE} $(TARLINT)
	date; ${MAKE} do_lint | grep -v '\.c:$$' | grep -v '^lint -I' >.lint.out 2>&1
	date; ${MAKE} tags
# hack -- adjust filenames for nfs access
#	sed 's!/users/awang!/net/beeblebrox/users/saldanha!g' tags >tags1
#	mv tags1 tags
#	${MAKE} depend
	${MAKE} help

require:
	@echo $(REQUIRE)

#-------------------------------------------------------------


# dummy targets
nothing:;

build header print uninstall debug debug-g debug-pg install.lint: nothing
	@echo no rule

_force:
