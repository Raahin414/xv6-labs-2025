
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *seps = " -\r\t\n./,";

int
is_sep(char c) {
  return strchr(seps, c) != 0;
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: sixfive file1 [file2 ...]\n");
    exit(0);
  }

  for (int fi = 1; fi < argc; fi++) {
    int fd = open(argv[fi], O_RDONLY);
    if (fd < 0) {
      printf("sixfive: cannot open %s\n", argv[fi]);
      continue;
    }

    int in_number = 0;
    long current = 0;
    char c;
    int n;

    while ((n = read(fd, &c, 1)) > 0) {
      if (!in_number) {
        if (c >= '0' && c <= '9') {
          in_number = 1;
          current = c - '0';
        } else {

        }
      } else {
        if (c >= '0' && c <= '9') {
          current = current * 10 + (c - '0');
        } else {

          if (current % 5 == 0 || current % 6 == 0) {
            printf("%ld\n", current);
          }
          in_number = 0;
          current = 0;
        }
      }
    }

    if (n == 0 && in_number) {
      if (current % 5 == 0 || current % 6 == 0) {
        printf("%ld\n", current);
      }
    }

    close(fd);
  }

  exit(0);
}
