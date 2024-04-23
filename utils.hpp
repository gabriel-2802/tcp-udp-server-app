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
#define MAX_SIZE 1500 // to be changed
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

struct __attribute__((__packed__)) incoming_udp_message {
	char topic[50];
	uint8_t data_type;
	char content[1501];
};

struct __attribute__((__packed__)) outgoing_udp_message {
    ip_address_t ip;
    port_t port;
    incoming_udp_message message;
};

int recv_all(int sockfd, void *buffer, size_t len) {
    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = (char *)buffer;

    while (bytes_remaining > 0) {
        bytes_received = recv(sockfd, buff, bytes_remaining, 0);
        if (bytes_received < 0) {
            return -1;  // Socket error
        } else if (bytes_received == 0) {
            return len - bytes_remaining;  // Connection closed, return bytes received so far
        }

        bytes_remaining -= bytes_received;
        buff += bytes_received;
    }

    return len;  // Return the total number of bytes supposed to be received
}

int send_all(int sockfd, void *buffer, size_t len) {
    size_t bytes_sent = 0;
    size_t bytes_remaining = len;
    char *buff = (char *)buffer;

    while (bytes_remaining > 0) {
        bytes_sent = send(sockfd, buff, bytes_remaining, 0);
        if (bytes_sent < 0) {
            return -1;  // Socket error, could potentially handle EAGAIN/EWOULDBLOCK for non-blocking sockets
        }

        bytes_remaining -= bytes_sent;
        buff += bytes_sent;
    }

    return len;  // Return the total number of bytes sent
}

vector<string> split(const string str, char delimiter) {
    stringstream ss(str);
    string token;
    vector<string> result;

    while (getline(ss, token, delimiter)) {
        result.push_back(token);
    }

    return result;
        
}

/* inspired from this https://leetcode.com/problems/wildcard-matching/solutions/4686045/simple-iterative-tabular-dp-solution/ */
bool is_match(const string s, const string p) {
    vector<string> path = split(s, '/');
    vector<string> pattern = split(p, '/');

    int n = path.size();
    int m = pattern.size();
    vector<vector<bool>> dp(n + 1, vector<bool>(m + 1, false));

    dp[0][0] = true;

    for (int i = 1; i <= m && pattern[i - 1] == "*"; ++i) {
        dp[0][i] = true;
    }

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (pattern[j - 1] == "*") {
                dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
            } else if (pattern[j - 1] == "+") {
                dp[i][j] = i > 0 && dp[i - 1][j - 1];
            } else if (pattern[j - 1] == path[i - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            }
        }
    }
 
    return dp[n][m];
}

void replace_new_line(char arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
        }
    }
}

