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
HashMap mimetype;
const char response_header_template[] = "HTTP/1.1 %d %s\r\n"
                                        "content-type: %s\r\n"
                                        "content-length: %zu\r\n"
                                        "server: custom-boi\r\n"
                                        "\r\n";

void serverInitMime();
unsigned long int hash(char *str);
int compare(char *a, char *b);
unsigned int serverRecv(const Socket *client);
HashMap serverParseHeader();
unsigned int serverSend(const Socket *client, HashMap *header);

void serverInitMime() {
  mimetype = hmapCreate((int (*)(void *, void *))compare,
                        (unsigned long int (*)(void *))hash, 20);
  hmapSet(&mimetype, "html", "text/html; charset=utf-8");
  hmapSet(&mimetype, "txt", "text/plain; charset=utf-8");
  hmapSet(&mimetype, "js", "application/javascript; charset=utf-8");

  hmapSet(&mimetype, "jpeg", "image/jpeg");
  hmapSet(&mimetype, "jpg", "image/jpeg");
  hmapSet(&mimetype, "png", "image/png");
}

unsigned long int hash(char *str) {
  unsigned int hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

int compare(char *a, char *b) { return !strcmp(a, b); }

unsigned int serverRecv(const Socket *client) {
  memset(serverBuffer, 0, BUFFER_SIZE);
  int size = recv(client->sock_fd, serverBuffer, BUFFER_SIZE, 0);
  return size;
}

HashMap serverParseHeader() {
  Vector lines = vectorCreate(20);
  char *line = strtok(serverBuffer, "\r\n");
  while (line) {
    vectorAppend(&lines, line);
    line = strtok(NULL, "\r\n");
  }

  HashMap header = hmapCreate((int (*)(void *, void *))compare,
                              (unsigned long int (*)(void *))hash, 20);

  hmapSet(&header, "method", strtok(serverBuffer, " "));
  hmapSet(&header, "url", strtok(NULL, " "));
  hmapSet(&header, "version", strtok(NULL, " "));

  for (size_t i = 1; i < lines.length; i++) {
    char *sep = strchr(lines.data[i], ':');
    *sep = 0;
    char *key = lines.data[i];
    char *value = sep + 1;
    hmapSet(&header, key, value);
  }

  vectorDestroy(&lines);
  return header;
}

unsigned int serverSend(const Socket *client, HashMap *header) {
  // read file and send it's contents.
  char *filename = hmapGet(header, "url") + 1;
  if (*filename == 0)
    filename = "index.html";

  FILE *response = fopen(filename, "r");
  if (response != NULL) {
    fseek(response, 0, SEEK_END);
    size_t size = ftell(response);
    rewind(response);
    unsigned char response_buffer[size];
    fread(response_buffer, sizeof(unsigned char), size, response);
    fclose(response);

    // generate response header.

    filename = strrchr(filename, '.');
    if (filename != NULL)
      filename++;
    else
      filename = "txt";

    char response_header[BUFFER_SIZE];
    size_t len = sprintf(response_header, response_header_template, 200, "OK",
                         (char *)hmapGet(&mimetype, filename), size);

    send(client->sock_fd, response_header, len, 0);
    send(client->sock_fd, response_buffer, size, 0);
  } else {
    char response_header[BUFFER_SIZE];
    size_t len = sprintf(response_header, response_header_template, 404,
                         "Not Found", "", 0l);
    send(client->sock_fd, response_header, len, 0);
  }

  return 0;
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
  serverInitMime();
}

void serverProcessClient() {
  Socket client;
  int address_len = sizeof(client.address);
  client.sock_fd = accept(server.sock_fd, (struct sockaddr *)&client.address,
                          (socklen_t *)&address_len);
  serverPrintInfo(&client);
  serverRecv(&client);
  HashMap header = serverParseHeader();
  serverSend(&client, &header);

  hmapDestroy(&header);
  close(client.sock_fd);
}

void serverStop() {
  hmapDestroy(&mimetype);
  close(server.sock_fd);
}
