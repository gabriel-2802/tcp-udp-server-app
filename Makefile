CFLAGS=-c -Wall -Werror -Wno-error=unused-variable -std=c++17
CC=g++

SUBSCRIBER_OBJ = client.o utils.o
SERVER_OBJ = server.o utils.o

EXE_SUBSCRIBER = subscriber
EXE_SERVER = server

all: $(EXE_SUBSCRIBER) $(EXE_SERVER)

$(EXE_SUBSCRIBER): $(SUBSCRIBER_OBJ)
	$(CC) -o $@ $^

$(EXE_SERVER): $(SERVER_OBJ)
	$(CC) -o $@ $^

client.o: client.cpp Client.hpp utils.h
	$(CC) $(CFLAGS) client.cpp -o $@

server.o: server.cpp Server.hpp utils.h DataBase.hpp
	$(CC) $(CFLAGS) server.cpp -o $@

utils.o: utils.cpp utils.h
	$(CC) $(CFLAGS) utils.cpp -o $@

clean:
	rm -f *.o $(EXE_SUBSCRIBER) $(EXE_SERVER) *.zip

zip:
	zip 321CA_Carauleanu_Valentin_Gabriel_Tema2.zip *.cpp *.hpp *.h Makefile README.txt

.PHONY: all clean
