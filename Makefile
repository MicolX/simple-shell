CFLAGS = -Wall -Wextra -Werror
EXE = sish
OBJS = main.o

all: ${EXE}

${EXE}: ${OBJS}
	cc ${CFLAGS} ${OBJS} -o ${EXE}

clean:
	rm -f ${EXE} *.o
