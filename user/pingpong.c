#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
  // init pipe
  int pipePtoC[2], pipeCtoP[2], status;
  pipe(pipePtoC);
  pipe(pipeCtoP);

  // fork
  int pid = fork();
  if(pid > 0) {
    /*parent should send a byte to the child*/
    int p_pid = getpid();
    close(pipePtoC[0]);
    write(pipePtoC[1], "Hello", 5);
    close(pipePtoC[1]);

    wait(&status);
    char read_buff[6] = {0};
    close(pipeCtoP[1]);
    read(pipeCtoP[0],read_buff, 5);
    if(*read_buff == 'H'){
      fprintf(1, "%d: received pong\n", p_pid);
    } else {
      fprintf(1, "ERROR!");
    }
    close(pipeCtoP[0]);

  } else if(pid == 0){
    int c_pid = getpid();
    char read_buff[6] = {0};
    close(pipePtoC[1]);
    read(pipePtoC[0],read_buff, 5);
    if(*read_buff == 'H'){
      fprintf(1, "%d: received ping\n", c_pid);
    } else {
      fprintf(1, "ERROR!");
    }
    close(pipePtoC[0]);

    close(pipeCtoP[0]);
    write(pipeCtoP[1], "Hello", 5);
    close(pipeCtoP[1]);
    exit(0);
  } else {
    write(2, "ERROR: fork unsuccess!", 22);
  }
  exit(0);
}
