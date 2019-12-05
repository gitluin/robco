CFLAGS= -Wall -g
LDADD=
LDFLAGS=

SRC= robco.c
OBJ= ${SRC:.c=.o}

all: robco

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}:

robco: ${OBJ}
	${CC} ${LDFLAGS} -o $@ ${OBJ} ${LDADD}

clean:
	rm -f *.o
