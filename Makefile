CC=clang

CLIENT_SRC=client.c
CLIENT_BIN=-o chat

SERVER_SRC=server.c usertable.c
SERVER_BIN=-o postman
SERVER_LIBS=-ljansson -lpthread

INCLUDE_PATH=-I/usr/local/include/
LIB_PATH=-L/usr/local/lib/

all: client server
	@echo "Done"

client:
	${CC} ${CLIENT_SRC} ${CLIENT_BIN} ${INCLUDE_PATH} ${LIB_PATH}

server:
	${CC} ${SERVER_SRC} ${SERVER_BIN} ${SERVER_LIBS} ${INCLUDE_PATH} ${LIB_PATH}
