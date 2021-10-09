#pragma once
/**
 * Function creates socket and initalizes socket required to establish
 * connection with the client.
 */
void serverInit(const char *address, unsigned short port);

/**
 * Function waits for client to make request and then processes the request.
 */
void serverProcessClient();

void serverStop();
