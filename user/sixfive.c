#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"


static char *seps = " -\r\t\n./,";

static int is_sep(char c) {
  for (char *p = seps; *p; p++) {
    if (*p == c) return 1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "anas tabba.\n");
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      fprintf(2, "sixfive: cannot open %s\n", argv[i]);
      continue;
    }

    char buf[1];
    char numbuf[32];
    int pos = 0;
    int ready = 1; // start-of-file is a separator, so ready to start numbers

    while (read(fd, buf, 1) == 1) {
      char c = buf[0];

      if (c >= '0' && c <= '9') {
        if (ready) {
          if (pos < (int)sizeof(numbuf) - 1) {
            numbuf[pos++] = c;
          }
          // if buffer overflows, we stil stay in number; excess digits ignored
        } else {
          // We are inside a non-separator word; ignore digits like the '6' in "xv6"
        }
      } else if (is_sep(c)) {
        if (pos > 0) {
          numbuf[pos] = '\0'; 
          int val = atoi(numbuf);
          if (val % 5 == 0 || val % 6 == 0) {
            printf("%d\n", val);
          }
          pos = 0;
        }
        ready = 1; 
      } else {

        pos = 0;
        ready = 0; 
      }
    }

    if (pos > 0) {
      numbuf[pos] = '\0'; 
      int val = atoi(numbuf);
      if (val % 5 == 0 || val % 6 == 0) {
        printf("%d\n", val);
      }
    }

    close(fd);
  }

  exit(0);
}