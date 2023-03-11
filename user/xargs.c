#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 3) {
    fprintf(2, "Usage: xargs command (argv)...\n");
    exit(1);
  }
  int totalargc = 0;
  char *totalargv[MAXARG];
  for(int i = 1; i < argc; i ++)
    totalargv[totalargc ++] = argv[i];

  char buf[512] = {0};
  char ch;

  int cnt = 0;
  while(read(0, &ch, 1)) {
    if(ch == 0) {
      buf[cnt++] = '\0';
      totalargv[totalargc] = (char*)malloc(cnt);
      memmove(totalargv[totalargc ++], buf, cnt);
      break;
    }
    if(ch == ' ' || ch == '\n') {
      if(cnt > 0) {
        buf[cnt ++] = '\0';
        totalargv[totalargc] = (char*)malloc(cnt);
        memmove(totalargv[totalargc], buf, cnt);
        cnt = 0;
        totalargc ++;
        if(totalargc >= MAXARG) {
          fprintf(2, "too many args\n");
          exit(1);
        }
      }
      continue;
    }
    buf[cnt ++] = ch;
    if(cnt >= 512) {
      fprintf(2, "Buffer overflow!");
      exit(1);
    }
  }

  if(fork() == 0) {
    exec(totalargv[0], totalargv);
  } else {
    wait(0);
  }
  exit(0);
}

