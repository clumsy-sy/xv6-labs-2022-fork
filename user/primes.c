#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
  int numArray[35], cnt = 0;
  for(int i = 2; i <= 35; i ++)
    numArray[cnt ++] = i;
  
  while(1) {
    if(cnt == 0) break;

    printf("prime %d\n", numArray[0]); // print primes
    // creat pipe
    int pip[2];
    pipe(pip);
    
    int pid = fork();
    if(pid < 0) {
      printf("ERROR fork!\n");
    } else if(pid > 0) {
      // parent
      close(pip[0]);
      for(int i = 1; i < cnt; i ++)
        if(numArray[i] % numArray[0] != 0)
          write(pip[1], &numArray[i], sizeof(int));
      close(pip[1]);
      wait(0);
      break; // important
    } else {
      // child
      cnt = 0;
      close(pip[1]);
      while(1) {
        int now, ret;
        ret = read(pip[0], &now, sizeof(int));
        if(ret == 0) break;
        numArray[cnt++] = now;
      }
      close(pip[0]);
    }
  }
  exit(0);
}
