#include "utils.h"

class Client {
    private:
        port_t port;
        socket_fd_t socket_fd;
        client_id_t id;
        vector<pollfd> poll_fds; // all sockets to be polled
        bool end_session = false;

    public:
        Client(char *argv[]) {
            int rc = sscanf(argv[3], "%hu", &port);
            DIE(rc != 1, "Given port is invalid");
            memset((void *)id.data(), '\0', MAX_ID);
            memcpy((void *)id.data(), argv[1], strlen(argv[1]));
            init_socket(argv);
        }

        ~Client() {
            for (auto& it : poll_fds) {
                close(it.fd);
            }
        }

        void run() {
            poll_fds.push_back({STDIN_FILENO, POLLIN}); // stdin
            poll_fds.push_back({socket_fd, POLLIN}); // the server socket

            size_t num_sockets = poll_fds.size();

            while (!end_session) {
                DIE(poll(poll_fds.data(), num_sockets, -1) < 0, "poll");
                
                // stdin
                if (poll_fds[0].revents & POLLIN) {
                    send_message();
                }

                // server socket
                if (poll_fds[1].revents & POLLIN) {
                    receive_message();
                }
               
            }
        }

    private:
        void init_socket(char *argv[]) {
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            DIE(socket_fd < 0, "socket");

            struct sockaddr_in serv_addr;
            socklen_t socket_len = sizeof(struct sockaddr_in);

            memset(&serv_addr, 0, socket_len);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);
            int rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
            DIE(rc <= 0, "inet_pton");

             // Set to 1 to disable Nagle's algorithm
            const int naggle_off = 1;
            DIE(setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &naggle_off, sizeof(int)) < 0, "setsockopt TCP_NODELAY");

            // connect to server
            rc = connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            DIE(rc < 0, "connect");

            // send the id to the server
            send(socket_fd, id.data(), strlen(id.data()), 0);
        }

        void send_message() {
            message_t message;
            memset(message, 0, sizeof(message_t));
            fgets(message, sizeof(message_t), stdin);

            if (!strncmp(message, EXIT, strlen(EXIT))) {
                end_session = true;
            } else if (!strncmp(message, SUBSCRIBE, strlen(SUBSCRIBE)) || !strncmp(message, UNSUBSCRIBE, strlen(UNSUBSCRIBE))) {
                send(socket_fd, message, strlen(message), 0);
                receive_confirmation();
            } else {
                cerr << "Invalid command\n";
            }
        }

        void receive_confirmation() {
            message_t message;
            memset(message, 0, sizeof(message_t));

            int rc = recv(socket_fd, message, sizeof(message_t), 0);
            DIE(rc < 0, "recv");

            if (rc == 0) {
                end_session = true;
                return;
            } else {
                stringstream ss;
                ss << message;
                cout << ss.str() << endl;
            }
        }

        void receive_message() {
            update_message_t message;
            memset(&message, 0, sizeof(update_message_t));

            int rc = recv_all(socket_fd, &message, sizeof(update_message_t));
            DIE(rc < 0, "recv");

            if (rc == 0) {
                end_session = true;
                return;
            } else {
                print_message(message);
            }
        }

        void print_message(outgoing_udp_message_t& message) {
            stringstream ss;
            ss << message.ip << ":" << message.port << " - " << message.message.topic << " - ";

            switch (message.message.data_type) {
                case INT: {
                    uint8_t sign = message.message.content[0];
                    uint32_t num;
                    memcpy(&num, message.message.content + 1, sizeof(uint32_t));
                    num = ntohl(num);
                    if (num == 0) {
                        ss << "INT - 0";
                        break;
                    }

                    ss << "INT - " << (sign ? "-" : "") << num;
                    break;
                } case SHORT_REAL: {
                    uint16_t short_real;
                    memcpy(&short_real, message.message.content, sizeof(uint16_t));
                    short_real = ntohs(short_real);
                    double real = static_cast<double>(short_real) / 100.0;
                    if (real == 0) {
                        ss << "SHORT_REAL - 0.00";
                        break;
                    }

                    ss << "SHORT_REAL - " << fixed << setprecision(2) << real;
                    break;
                } case FLOAT: {
                    uint8_t sign_f = message.message.content[0];
                    uint32_t num_f;
                    memcpy(&num_f, message.message.content + 1, sizeof(uint32_t));
                    num_f = ntohl(num_f);
                    uint8_t power;
                    memcpy(&power, message.message.content + 5, sizeof(uint8_t));

                    double number = static_cast<double>(num_f);
                    for (int i = 0; i < power; i++) {
                        number /= 10.0;
                    }
                    
                    if (num_f == 0) {
                        ss << "FLOAT - 0";
                        break;
                    }
                    ss << "FLOAT - " << (sign_f ? "-" : "") << std::fixed << std::setprecision(power) << number;
                    break;
                } case STRING: {
                    ss << "STRING - " << message.message.content;
                    break;
                } default: {
                    cerr << "Invalid data type\n";
                    return;
                }
            }

            cout << ss.str() << endl;


        }
    
    

};