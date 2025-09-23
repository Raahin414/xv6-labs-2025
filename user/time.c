#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf("Usage: time command [args...]\n");
    exit(1);
  }

  int start = uptime();

  int pid = fork();
  if(pid < 0){
    printf("time: fork failed\n");
    exit(1);
  }

  if(pid == 0){
    exec(argv[1], argv+1);
    printf("time: exec failed\n");
    exit(1);
  } else {
    wait(0);
    int end = uptime();
    int dur = end - start;
    printf("%d\n", dur);   // ✅ fixed
  }

  exit(0);
}

