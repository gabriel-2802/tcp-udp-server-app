CFLAGS=-c -Wall -Werror -Wno-error=unused-variable -std=c++17
CC=g++

SUBSCRIBER_OBJ = client.o
SERVER_OBJ = server.o
EXE_SUBSCRIBER = subscriber
EXE_SERVER = server

all: $(EXE_SUBSCRIBER) $(EXE_SERVER)

$(EXE_SUBSCRIBER): $(SUBSCRIBER_OBJ)
	$(CC) -o $@ $^

$(EXE_SERVER): $(SERVER_OBJ)
	$(CC) -o $@ $^

client.o: client.cpp Client.hpp utils.hpp
	$(CC) $(CFLAGS) client.cpp -o $@

server.o: server.cpp Server.hpp utils.hpp DataBase.hpp
	$(CC) $(CFLAGS) server.cpp -o $@

clean:
	rm -f *.o $(EXE_SUBSCRIBER) $(EXE_SERVER)

.PHONY: all clean
