-include config.mk
LFLAGS+= -lSDL2 -lepoxy -lGL -lm
OBJ= lex.gen.o parse.gen.o driver.o compile.o sdl.o
PROG= graphcalc
.PHONY: all clean

all: ${PROG}

${PROG}: ${OBJ}
	${CC} ${CFLAGS} ${LFLAGS} -o ${PROG} ${OBJ}

lex.gen.c: lex.re.c parse.gen.h
	re2c -o lex.gen.c lex.re.c

parse.gen.c parse.gen.h: parse.lem
	# seriously sucks you cant do -o w/ lemon :/
	lemon -q parse.lem
	mv parse.h parse.gen.h
	mv parse.c parse.gen.c

clean:
	rm -rf *.o *.gen.* ${PROG}
