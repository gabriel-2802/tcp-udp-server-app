#include "Server.hpp"


int main(int argv, char* argc[]) {
   DIE(argv != 2, "Not enough arguments");
   setvbuf(stdout, NULL, _IONBF, BUFSIZ);


    Server *server = new Server(argc);
    server->run();
    delete server;

   return 0;
}