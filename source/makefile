EXE    = m
OBJDIR =

# compiler and linker

TGT = +aos68k
CC = vc $(TGT)
LD = vc $(TGT)
CFLAGS = -c99 -lamiga -lauto -DNDEBUG

all : $(EXE)

x.o : x.c
	$(CC) -c -o x.o x.c

main.o : main.c
    $(CC) -c -o main.o main.c

rooms.o : rooms.c
    $(CC) -c -o rooms.o rooms.c

GOBJS = main.o rooms.o
WOBJS = x.o

$(EXE) : $(GOBJS) $(WOBJS)
	$(LD) $(CFLAGS) $(GOBJS) $(WOBJS) -o $(EXE)

test: $(GOBJS) $(WOBJS)
	-delete m
	$(LD) $(CFLAGS) $(GOBJS) $(WOBJS) -o $(EXE)
    $(EXE)

clean:
	delete $(OBJS) $(EXE)
