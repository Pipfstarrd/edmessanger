CC=clang

CLIENT_SRC=client.c
CLIENT_BIN=-o chat
CLIENT_LIBS=-ljansson -lpthread
CLIENT_OPTIONS=-g

SERVER_SRC=server.c usertable.c eventlist.c api.c
SERVER_BIN=-o postman
SERVER_LIBS=-ljansson -lpthread
SERVER_OPTIONS=-g

INCLUDE_PATH=-I/usr/local/include/
LIB_PATH=-L/usr/local/lib/

all: client server
	@echo "Done"

client:
	${CC} ${CLIENT_SRC} ${CLIENT_BIN} ${CLIENT_LIBS} ${CLIENT_OPTIONS} ${INCLUDE_PATH} ${LIB_PATH}

server:
	${CC} ${SERVER_SRC} ${SERVER_BIN} ${SERVER_LIBS} ${SERVER_OPTIONS} ${INCLUDE_PATH} ${LIB_PATH}
