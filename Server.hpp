#include "utils.h"
#include "DataBase.hpp"
#define POLLS 100

class Server {
    private:
        // connection data
        port_t port;
        socket_fd_t tcp_fd;
        socket_fd_t udp_fd;
        socket_fd_t stdio_fd;
        vector<pollfd> poll_fds; // all sockets to be polled

        // administration data
        DataBase db;
        
        bool end_session = false;

    public:
        Server(char *argv[]) {
            int rc = sscanf(argv[1], "%hu", &port);
            DIE(rc != 1, "Given port is invalid");
            init_tcp(argv);
            init_udp(argv);
            init_stdio();
        }

        ~Server() {
           for (auto& it : poll_fds) {
               close(it.fd);
           }
        }

        void run() {
            DIE(listen(tcp_fd, POLLS) < 0, "listen");
            poll_fds.push_back({tcp_fd, POLLIN});
            poll_fds.push_back({udp_fd, POLLIN});
            poll_fds.push_back({stdio_fd, POLLIN});
            
            while (!end_session) {
                DIE(poll(poll_fds.data(), poll_fds.size(), -1) < 0, "poll");

                for (auto& poll_fd : poll_fds) {
                    if (poll_fd.revents & POLLIN) {
                        if (poll_fd.fd == tcp_fd) {
                            handle_new_tcp_connection();
                        } else if (poll_fd.fd == udp_fd) {
                            handle_new_udp_connection();
                        } else if (poll_fd.fd == stdio_fd) {
                            handle_stdin();
                        } else {
                            handle_data_from_client(poll_fd);
                        }
                    }
                    if (end_session) {
                        break;
                    }
                }
            }

        }

    private:
        void init_tcp(char *argv[]) {
            tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
            DIE(tcp_fd < 0, "socket tcp");

            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);

            int rc = inet_pton(AF_INET, LOCAL_NETWORK, &serv_addr.sin_addr.s_addr);
            DIE(rc <= 0, "inet_pton");

            const int naggle_off = 1;  // Set to 1 to disable Nagle's algorithm
            DIE(setsockopt(tcp_fd, IPPROTO_TCP, TCP_NODELAY, &naggle_off, sizeof(int)) < 0, "setsockopt TCP_NODELAY");

            const int enable = 1;
            DIE(setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0, "setsockopt");
            
            rc = bind(tcp_fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
            DIE(rc < 0, "bind");
        }

        void init_udp(char *argv[]) {
            udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
            DIE(udp_fd < 0, "socket udp");

            struct sockaddr_in serv_addr;
            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);
            int rc = inet_pton(AF_INET, LOCAL_NETWORK, &serv_addr.sin_addr.s_addr);
            DIE(rc <= 0, "inet_pton");

            const int enable = 1;
            DIE(setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0, "setsockopt");

            rc = bind(udp_fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
            DIE(rc < 0, "bind");
        }

        void init_stdio() {
            stdio_fd = STDIN_FILENO;
        }

        void handle_new_tcp_connection() {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            socket_fd_t client_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &client_len);
            DIE(client_fd < 0, "accept");

            client_id_t client_id;
            message_t buff;
            memset((void *)buff, '\0', MAX_SIZE);
            DIE(recv(client_fd, (void *)buff, MAX_ID, 0) < 0, "recv");
            
            client_id.assign(buff);
            // check if client_id is already used
           if (db.is_client_online(client_id)) {
                // Client C1 already connected.
                cout << "Client " << client_id.data() << " already connected.\n";
                close(client_fd);
                return;
            }

            // add client to the poll and database
            poll_fds.push_back({client_fd, POLLIN});
            db.add_new_client(client_id, client_fd);

            // print client info
            string ip = inet_ntoa(client_addr.sin_addr);
            int port = ntohs(client_addr.sin_port);
            cout << "New client " << client_id.data() << " connected from " << ip << ":" << port << ".\n";
        }


        void handle_new_udp_connection() {
            incoming_udp_message_t message;
            memset(&message, 0, sizeof(message));

            // receive message from udp client
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int rc = recvfrom(udp_fd, &message, sizeof(message), 0, (struct sockaddr *)&client_addr, &client_len);
            DIE(rc < 0, "recvfrom");
            
            // forward message to subscribers
            outgoing_udp_message_t forward_message;
            memset(&forward_message, 0, sizeof(forward_message));
            memcpy(forward_message.message.topic, message.topic, sizeof(message.topic));
            forward_message.message.data_type = message.data_type;
            memcpy(forward_message.message.content, message.content, sizeof(message.content));
            
            // add source ip and port
            memcpy(forward_message.ip, inet_ntoa(client_addr.sin_addr), sizeof(forward_message.ip));
            forward_message.port = ntohs(client_addr.sin_port);

            send_topic_to_subscribers(forward_message);
        }

        void send_topic_to_subscribers(outgoing_udp_message_t message) {
            // find subscribers for the topic
            topic_t topic = message.message.topic;
            set<socket_fd_t> subscribers = db.get_subscribers(topic);

            // send message to subscribers
            for (auto& subscriber : subscribers) {
                send_all(subscriber, &message, sizeof(message));
            }
        }

        void handle_data_from_client(pollfd client) {
            message_t message;
            memset(message, 0, sizeof(message));
        
            // get message from client
            int rc = recv(client.fd, &message, sizeof(message), 0);
            if (rc == 0) {
                handle_client_disconnection(client);
            } else if (!strncmp(message, SUBSCRIBE, strlen(SUBSCRIBE))) {
                handle_subscribe(client, message + strlen(SUBSCRIBE));
            } else if (!strncmp(message, UNSUBSCRIBE, strlen(UNSUBSCRIBE))) {
                handle_unsubscribe(client, message + strlen(UNSUBSCRIBE));
            } else {
                cerr << "Invalid command\n";
            }
        }

        void handle_subscribe(pollfd client, message_t message) {
            replace_new_line(message, strlen(message));

            // get topics from message as a vector
            vector<topic_t> topics = split(message, ' ');
            // get client_id from socket
            client_id_t client_id = db.get_id_from_socket(client.fd);
            // add topics to client subscriptions
            for (auto& topic : topics) {
                db.add_subscription(client_id, topic);
            }

            // send confirmation to client
            message_t response;
            memset(response, 0, sizeof(response));
            memcpy(response, SUBSCRIBE_REPLY, strlen(SUBSCRIBE_REPLY));
            memcpy(response + strlen(SUBSCRIBE_REPLY), message, strlen(message));
            send(client.fd, response, strlen(response), 0);
        }

        void handle_unsubscribe(pollfd client, message_t message) {
            replace_new_line(message, strlen(message));

            vector<topic_t> topics = split(message, ' ');
            client_id_t client_id = db.get_id_from_socket(client.fd);
            for (auto& topic : topics) {
                db.remove_subscription(client_id, topic);
            }

            // send confirmation to client
            message_t response;
            memset(response, 0, sizeof(response));
            memcpy(response, UNSUBSCRIBE_REPLY, strlen(UNSUBSCRIBE_REPLY));
            memcpy(response + strlen(UNSUBSCRIBE_REPLY), message, strlen(message));
            send(client.fd, response, strlen(response), 0);
        }

        void handle_client_disconnection(pollfd client) {
            // get client_id
            client_id_t client_id = db.get_id_from_socket(client.fd);
            db.remove_online_client(client_id);

            // remove client from online clients
            for (auto it = poll_fds.begin(); it != poll_fds.end(); it++) {
                if (it->fd == client.fd) {
                    poll_fds.erase(it);
                    break;
                }
            }

            cout << "Client " << client_id.data() << " disconnected.\n";
        }

        void handle_stdin() {
            message_t message;
            memset(message, 0, sizeof(message));
            fgets(message, sizeof(message), stdin);

            if (!strncmp(message, EXIT, strlen(EXIT))) {
                end_session = true;
            } else {
                cerr << "Invalid command\n";
            }
        }  
};