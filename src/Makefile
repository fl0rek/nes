IDIR = ../include
CC=gcc
CFLAGS= -I$(IDIR)

ODIR=../obj
LDIR =../lib

LIBS=-lpanel -lmenu

_DEPS = avl.h marshall.h backend.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = avl.o marshall.o backend.o frontend.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

notevensql: $(OBJ)
	gcc -o ../$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

