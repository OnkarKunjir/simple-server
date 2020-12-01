#pragma once
#include <netinet/in.h>

struct ConnectionInfo {
    int file_descriptor;
    struct sockaddr_in address;
};

class SimpleServer {
    private:
        int file_descriptor, backlog;
        struct sockaddr_in address;

        ConnectionInfo accept_connection();
        int process_GET(const ConnectionInfo &connection, const char* request_header, int header_len);
        int process_request(const ConnectionInfo &connection, int buffer_size);
        void send_message(const ConnectionInfo &connection, const char* message, int message_len);
        void print_connection_info(const ConnectionInfo &connection);

    public:
        static void hex_dump(const char* buffer, int buffer_len);
        SimpleServer(const char* ip_address, int port, int backlog);
        int init();
        int serve();
        ~SimpleServer();
};
