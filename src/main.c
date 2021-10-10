#include "server.h"
#include <signal.h>
#include <stdio.h>

int serve = 1;
void terminate(int signal) { serve = 0; }

int main() {
  signal(SIGINT, terminate);

  serverInit("127.0.0.1", 8080);
  while (serve) {
    serverProcessClient();
  }
  serverStop();
  return 0;
}
