#include "server.h"

// networking headers.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "container/hashmap.h"
#include "container/vector.h"

#define BUFFER_SIZE 1024 * 8
#define PORT 8080
#define BACKLOG 5

typedef struct Socket {
  int sock_fd;
  struct sockaddr_in address;
} Socket;

char serverBuffer[BUFFER_SIZE];
Socket server;

/**
 * Function to receive the infromation form client. Stores into serverBuffer
 */
unsigned int serverRecv();
void serverParseHeader();

unsigned int serverRecv(const Socket *client) {
  memset(serverBuffer, 0, BUFFER_SIZE);
  int size = recv(client->sock_fd, serverBuffer, BUFFER_SIZE, 0);
  return size;
}

void serverParseHeader() {
  char *header_end = strstr(serverBuffer, "\r\n\r\n");
  unsigned int header_len = header_end - serverBuffer;

  Vector lines = vectorCreate(20);
  char *line = strtok(serverBuffer, "\r\n");
  while (line) {
    vectorAppend(&lines, line);
    line = strtok(NULL, "\r\n");
  }

  for (size_t i = 1; i < lines.length; i++) {
    char *sep = strchr(lines.data[i], ':');
    *sep = 0;
    char *key = lines.data[i];
    char *value = sep + 1;
    printf("%s : %s\n", key, value);
  }

  vectorDestroy(&lines);
}

// functions from header file.
void serverPrintInfo(const Socket *connection) {
  printf("Connection from %s:%d\n", inet_ntoa(connection->address.sin_addr),
         ntohs(connection->address.sin_port));
}

void serverInit(const char *address, unsigned short port) {

  if ((server.sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Failed to create socket\n");
    exit(EXIT_FAILURE);
  }

  // set socket options.
  int flag = 1;
  if (setsockopt(server.sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &flag,
                 sizeof(flag))) {
    perror("Failed to set socket options\n");
    exit(EXIT_FAILURE);
  }

  // ipv4
  server.address.sin_family = AF_INET;
  server.address.sin_port = htons(port);
  inet_aton(address, &server.address.sin_addr);
  memset(server.address.sin_zero, 0, sizeof(server.address.sin_addr));

  if (bind(server.sock_fd, (const struct sockaddr *)&server.address,
           sizeof(server.address)) == -1) {
    perror("Failed to bind socket\n");
  }

  if (listen(server.sock_fd, BACKLOG) == -1) {
    perror("Failed to listend\n");
  }
}

void serverProcessClient() {
  Socket client;
  int address_len = sizeof(client.address);
  client.sock_fd = accept(server.sock_fd, (struct sockaddr *)&client.address,
                          (socklen_t *)&address_len);
  serverPrintInfo(&client);
  serverRecv(&client);
  serverParseHeader();
  close(client.sock_fd);
}

void serverStop() { close(server.sock_fd); }
