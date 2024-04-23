#include "utils.hpp"

class DataBase {
    public:
        // used to keep track of all clients and their subscriptions
       unordered_map<client_id_t, set<topic_t>> client_subscriptions;

        // used for online clients only
        set<client_id_t> online_clients;
        unordered_map<socket_fd_t, client_id_t> socket_id_clients;
        unordered_map<client_id_t, socket_fd_t> id_socket_clients;

    public:
        DataBase() = default;
        ~DataBase() = default;

        void add_new_client(client_id_t id, socket_fd_t socket_fd) {
            online_clients.insert(id);

            socket_id_clients[socket_fd] = id;
            id_socket_clients[id] = socket_fd;
        }

        void remove_online_client(client_id_t id) {
            online_clients.erase(id);

            socket_fd_t socket_fd = id_socket_clients[id];
            socket_id_clients.erase(socket_fd);
            id_socket_clients.erase(id);
        }

        client_id_t get_id_from_socket(socket_fd_t socket_fd) {
            return socket_id_clients[socket_fd];
        }

        socket_fd_t get_socket_from_id(client_id_t id) {
            return id_socket_clients[id];
        }

        bool is_client_online(client_id_t id) {
            return online_clients.find(id) != online_clients.end();
        }

        void add_subscription(client_id_t id, topic_t topic) {
            client_subscriptions[id].insert(topic);
        }

        void remove_subscription(client_id_t id, topic_t topic) {
            client_subscriptions[id].erase(topic);
        }

        set<socket_fd_t> get_subscribers(topic_t topic) {
            set<client_id_t> result;
            for (auto& [id, topics] : client_subscriptions) {
                if (!is_client_online(id)) {
                    continue;
                }

                for (auto& t : topics) {
                    if (is_match(t, topic)) {
                        result.insert(id);
                        break;
                    }
                }
            }

            set<socket_fd_t> subscribers_fds;
            for (auto& id : result) {
                subscribers_fds.insert(id_socket_clients[id]);
            }

            return subscribers_fds;
        }

        set<topic_t> get_subscriptions(client_id_t id) {
            return client_subscriptions[id];
        }

        void print_users_subscriptions() {
            for (auto& [id, topics] : client_subscriptions) {
                cout << "Client " << id << " is subscribed to: ";
                for (auto& t : topics) {
                    cout << t << " ";
                }
                cout << endl;
            }
        }
};