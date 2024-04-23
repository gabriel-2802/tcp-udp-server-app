#include "utils.h"

int recv_all(int sockfd, void *buffer, size_t len) {
    size_t bytes_received = 0;
    size_t bytes_remaining = len;
    char *buff = (char *)buffer;

    while (bytes_remaining > 0) {
        bytes_received = recv(sockfd, buff, bytes_remaining, 0);
        if (bytes_received < 0) {
            return -1;
        } else if (bytes_received == 0) {
            return len - bytes_remaining; 
        }

        bytes_remaining -= bytes_received;
        buff += bytes_received;
    }

    return len;
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

vector<string> split(string str, char delimiter) {
    stringstream ss(str);
    string token;
    vector<string> result;

    while (getline(ss, token, delimiter)) {
        result.push_back(token);
    }

    return result;
        
}

bool is_match(string s, string p) {
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
