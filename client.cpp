#include "Client.hpp"

int main(int argc, char* argv[]) {
   DIE(argc != 4, "Not enough arguments");
   setvbuf(stdout, NULL, _IONBF, BUFSIZ);


    Client *client = new Client(argv);
    client->run();
    delete client;


   return 0;
}