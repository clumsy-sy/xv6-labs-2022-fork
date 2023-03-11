#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int times = 0;

  if(argc <= 1 || argc > 2) {
    fprintf(2, "usage: sleep pattern [times]\n");
    exit(1);
  }
  times = atoi(argv[1]);
  sleep(times);
  exit(0);
}
