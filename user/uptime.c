#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
  int t = uptime();        
  printf("%d\n", t);
  exit(0);
}