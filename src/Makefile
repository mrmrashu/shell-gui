CC  = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0`
APP = main
LIBS = `pkg-config --libs gtk+-3.0`

${APP}: ${APP}.c style.css source.c scanner.c parser.c node.c output.c executor.c prompt.c
	${CC} ${CFLAGS} -o ${APP} source.c scanner.c parser.c node.c output.c executor.c prompt.c ${APP}.c ${LIBS} 

clear: 
	rm ${APP}