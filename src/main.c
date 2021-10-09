#include "server.h"
#include <stdio.h>

int main() {
  serverInit("127.0.0.1", 8080);
  serverProcessClient();
  serverStop();
  return 0;
}
