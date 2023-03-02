EXE    = mcc
OBJDIR =

# compiler and linker

TGT = +aos68k
CC = vc $(TGT)
LD = vc $(TGT)
CFLAGS = -c99 -g -debug=3 -lamiga -lauto -DNDEBUG

all : $(EXE)

compiler.o : compiler.c
	$(CC) -c -o compiler.o compiler.c

globals.o : globals.c
    $(CC) -c -o globals.o globals.c

rooms.o : rooms.c
    $(CC) -c -o rooms.o rooms.c

tokenizer.o : tokenizer.c
    $(CC) -c -o tokenizer.o tokenizer.c

vocab.o : vocab.c
    $(CC) -c -o vocab.o vocab.c

text.o : text.c
    $(CC) -c -o text.o text.c

OBJS = compiler.o globals.o rooms.o tokenizer.o vocab.o text.o

$(EXE) : $(OBJS)
	$(LD) $(CFLAGS) $(OBJS) -o $(EXE)

test: $(OBJS)
	-delete $(EXE)
	$(LD) $(CFLAGS) $(OBJS) -o $(EXE)
    $(EXE)

clean:
	delete $(OBJS) $(EXE)
#
