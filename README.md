## TCP-UDP Server Overview

### Structure Overview
- **Client.hpp**: Defines the client class.
- **Server.hpp**: Defines the server class.
- **DataBase.hpp**: Manages the database functionalities used by the server.
- **utils.h**: Contains utility functions and macro definitions.
- **client.cpp**: Implements the client program.
- **server.cpp**: Implements the server program.

### Development Process
The basis of the implementation is the 7th laboratory tasks.

#### 1. **Server Functionality**
- **Socket Initialization**: The server sets up 3 sockets for TCP, UDP, and STDIN, adding them to a `poll_fds` vector that monitors file descriptor activity
- **Connection Listening**: Actively listens for incoming TCP connections.
- **Activity Handling**: Based on the `POLLIN` event, the server acts on the following:
    - **TCP Connection Requests**: Accepts new connections and integrates new clients into the `poll_fds` vector.
    - **UDP Messages**: Receives messages and dispatches them to TCP clients subscribed to the relevant topics.
    - **STDIN Commands**: Processes standard input commands appropriately.
- **Client Management**:
    - On new TCP connections, expects a client ID, rejecting invalid IDs, closing the connection, and printing an error message.
    - For new valid connections, registers the client in the database
- **Message Routing**:
    - Forwards UDP messages to all TCP clients subscribed to the relevant topics using the DataBase.
    - Processes TCP client messages to execute commands: EXIT, SUBSCRIBE, and UNSUBSCRIBE.

#### 2. **Client Operations**
- **TCP Socket Initialization**: Connects to the server using a designated TCP socket.
- **ID Transmission**: Upon connection, sends its ID to the server.
- **Command Processing**: Reads and sends commands from STDIN to the server: subscribe, unsubscribe, and exit; all other commands are ignored and an error message is displayed.
- **Message Handling**: Receives and displays server messages on STDOUT according to a predefined format.

#### 3. **DataBase Integration**
- **Client and Subscription Storage**:
    - Maintains a set of online clients by ID.
    - Maps IDs to sockets for online client tracking.
    - Maps sockets to IDs for reverse lookup.
    - Manages subscriptions with a mapping from IDs to sets of topics.
- **Efficient Data Access**: Utilizes sets and unordered maps for O(1) access to client and subscription data.

#### 4. **Communication Protocol**
- **Structured Communication**:
    - Upon connection, the client sends the server the size of its ID, followed by the ID itself.
    - The server validates the client and confirms the connection to STDOUT
    - Clients issue commands (EXIT, SUBSCRIBE, UNSUBSCRIBE) which are acknowledged by the server; before each message, the client sends the size of the message to the server to ensure the server receives the message in its entirety.
    - the servers acknowledge the message by sending a confirmation message to the client, preceded by the size of the message.
- **Message Forwarding**:
    - Server forwards topic-based messages to subscribed clients.
    - the server receives `incoming_udp_message_t` messages from the UDP socket and forwards them to the relevant TCP clients.
    - the forwarding process is executed according to a predefined format:
        - the servers sends sizeof(`message_metadata_t`) bytes containing the udp client's information, the topic, the data type and size of the message.
        - the server sends the message itself to the client.
- This ensures the messages are sent and received correctly, preventing buffer overflows and data loss or data corruption.
- Moreover, the method of sending the message size before the message itself ensures that the message is received correctly and in its entirety by the receiving party.