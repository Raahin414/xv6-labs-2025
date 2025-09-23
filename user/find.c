#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"

char *fmtname(char *path) {
  static char buf[DIRSIZ+1];
  char *p;

  for(p = path + strlen(path); p >= path && *p != '/'; p--) ;
  p++;
  if(strlen(p) >= DIRSIZ) return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

void do_exec_on_file(int argc_exec, char *exec_argv[], char *file) {
  char *argv[MAXARG];
  int i, j = 0;
  int has_placeholder = 0;

  for (i = 0; i < argc_exec && j < MAXARG-1; i++) {
    if (strcmp(exec_argv[i], "{}") == 0) {
      argv[j++] = file;
      has_placeholder = 1;
    } else {
      argv[j++] = exec_argv[i];
    }
  }
  if (!has_placeholder && j < MAXARG-1) {
    argv[j++] = file;
  }
  argv[j] = 0;

  if (fork() == 0) {
    exec(argv[0], argv);
    printf("find: exec failed for %s\n", argv[0]);
    exit(1);
  }
  wait(0);
}

void find(char *path, char *name, int do_exec, int argc_exec, char *exec_argv[]) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0) {
    printf("find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    printf("find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type == T_FILE) {
    if (strcmp(fmtname(path), name) == 0) {
      if (do_exec) do_exec_on_file(argc_exec, exec_argv, path);
      else printf("%s\n", path);
    }
    close(fd);
    return;
  }

  if (st.type == T_DIR) {
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      printf("find: path too long\n");
      close(fd);
      return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0) continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, name, do_exec, argc_exec, exec_argv);
    }
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: find <startdir> <name> [-exec cmd [args...]]\n");
    exit(1);
  }

  if (argc >= 4 && strcmp(argv[3], "-exec") == 0) {
    char *exec_argv[MAXARG];
    int argc_exec = 0;
    for (int i = 4; i < argc && argc_exec < MAXARG-1; i++) {
      exec_argv[argc_exec++] = argv[i];
    }
    exec_argv[argc_exec] = 0;

    find(argv[1], argv[2], 1, argc_exec, exec_argv);
  } else {
    find(argv[1], argv[2], 0, 0, 0);
  }

  exit(0);
}

