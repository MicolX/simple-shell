CFLAGS = -g -Wall -Wextra -Werror
EXE = sish
OBJS = main.o util.o

all: ${EXE}

${EXE}: ${OBJS}
	cc ${CFLAGS} ${OBJS} -o ${EXE}

clean:
	rm -f ${EXE} *.o
