#pragma once
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <array>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <iomanip>
using namespace std;

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)
#define LOCAL_NETWORK "127.0.0.1"
#define MAX_IP_SIZE 16
#define MAX_ID 15
#define MAX_SIZE 1500
#define EXIT "exit"
#define SUBSCRIBE "subscribe "
#define UNSUBSCRIBE "unsubscribe "
#define SUBSCRIBE_REPLY "Subscribed to topic "
#define UNSUBSCRIBE_REPLY "Unsubscribed from topic "

typedef uint16_t port_t;
typedef int socket_fd_t;
typedef string client_id_t;
typedef string topic_t;
typedef char message_t[MAX_SIZE];
typedef char ip_address_t[MAX_IP_SIZE];

enum DataType : uint8_t {
    INT,
    SHORT_REAL,
    FLOAT,
    STRING
};

// avoid padding in struct
struct __attribute__((__packed__)) incoming_udp_message_t {
	char topic[51];
	uint8_t data_type;
	char content[1501];
};

// avoid padding in struct
struct __attribute__((__packed__)) outgoing_udp_message_t {
    ip_address_t ip;
    port_t port;
    incoming_udp_message_t message;
};

struct __attribute__((__packed__)) message_metadata_t {
	ip_address_t ip;
	port_t port;
	char topic[51];
	uint8_t data_type;
};

// for the server, the message it outgoing
// for the client, the message is incoming as an update to a topic
typedef outgoing_udp_message_t update_message_t;

int recv_all(int sockfd, void *buffer, size_t len);

int send_all(int sockfd, void *buffer, size_t len);

vector<string> split(string str, char delimiter);

/* inspired from this https://leetcode.com/problems/wildcard-matching/solutions/4686045/simple-iterative-tabular-dp-solution/ */
bool is_match(string s, string p);

void replace_new_line(char arr[], int size);
