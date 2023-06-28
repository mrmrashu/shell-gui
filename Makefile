CC  = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0`
APP = main
LIBS = `pkg-config --libs gtk+-3.0`

${APP}: ${APP}.c style.css
	${CC} ${CFLAGS} -o ${APP} ${APP}.c ${LIBS} 

clear: 
	rm ${APP}