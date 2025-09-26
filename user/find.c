#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

static int do_exec;
static char *exec_args[MAXARG];
static int exec_args_count;

static int use_regex = 0;

static int match_here(char*, char*);
static int match_star(int, char*, char*);

static int
match(char *pattern, char *text)
{
  if(pattern[0] == '^')
    return match_here(pattern+1, text);

  do {
    if(match_here(pattern, text))
      return 1;
  } while(*text++ != '\0');

  return 0;
}

static int
match_here(char *pattern, char *text)
{
  if(pattern[0] == '\0') return 1;
  if(pattern[1] == '*') return match_star(pattern[0], pattern+2, text);
  if(pattern[0] == '$' && pattern[1] == '\0') return *text == '\0';
  if(*text != '\0' && (pattern[0]=='.' || pattern[0]==*text))
    return match_here(pattern+1, text+1);

  return 0;
}

static int
match_star(int c, char *pattern, char *text)
{
  do {
    if(match_here(pattern, text)) return 1;
  } while(*text != '\0' && (*text++ == c || c == '.'));

  return 0;
}

static void
run_on_file(const char *file_path)
{
  char *argv[MAXARG];
  int arg_count = 0;

  for(int i = 0; i < exec_args_count && arg_count < MAXARG-1; i++)
    argv[arg_count++] = exec_args[i];

  if(arg_count < MAXARG-1) argv[arg_count++] = (char *)file_path;
  argv[arg_count] = 0;

  int pid = fork();
  if(pid < 0) {
    fprintf(2, "find: fork failed\n");
    return;
  }

  if(pid == 0) {
    exec(argv[0], argv);
    fprintf(2, "find: exec %s failed\n", argv[0]);
    exit(1);
  }

  wait(0);
}

static void
search(const char *path, const char *target)
{
  int fd = open(path, O_RDONLY);
  if(fd < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  struct stat st;
  if(fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if(st.type == T_FILE) {
    const char *base_name = path;
    for(const char *p = path; *p; p++)
      if(*p == '/') base_name = p + 1;

    int matched = 0;
    if(use_regex)
      matched = match((char *)target, (char *)base_name);
    else
      matched = strcmp((char *)base_name, (char *)target) == 0;

    if(matched) {
      if(do_exec) run_on_file(path);
      else printf("%s\n", path);
    }

  } else if(st.type == T_DIR) {
    char buffer[512];
    int path_len = strlen(path);
    if(path_len + 1 + DIRSIZ + 1 > sizeof(buffer)) {
      fprintf(2, "find: path too long: %s\n", path);
      close(fd);
      return;
    }

    struct dirent de;
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
      if(de.inum == 0) continue;

      char name[DIRSIZ+1];
      memmove(name, de.name, DIRSIZ);
      name[DIRSIZ] = 0;

      if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        continue;

      strcpy(buffer, path);
      buffer[path_len] = '/';
      buffer[path_len+1] = '\0';
      strcpy(buffer + path_len + 1, name);

      search(buffer, target);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  use_regex = 1;

  if(argc < 2) {
    fprintf(2, "usage: find <start-path> <name|pattern> [-E | -F] [-exec <cmd> [args...]]\n");
    exit(1);
  }

  do_exec = 0;
  exec_args_count = 0;
  use_regex = 0;

  const char *start_path = argv[1];
  const char *target_name = 0;

  for(int i = 2; i < argc; i++) {
    if(strcmp(argv[i], "-E") == 0) {
      use_regex = 1;
      if(i+1 < argc && argv[i+1][0] != '-') {
        target_name = argv[i+1];
        i++;
      }
      continue;
    }

    if(strcmp(argv[i], "-F") == 0) {
      use_regex = 0;
      if(i+1 < argc && argv[i+1][0] != '-') {
        target_name = argv[i+1];
        i++;
      }
      continue;
    }

    if(strcmp(argv[i], "-exec") == 0) {
      do_exec = 1;
      if(i+1 >= argc) {
        fprintf(2, "find: -exec requires a command\n");
        exit(1);
      }
      for(int j = i+1; j < argc && exec_args_count < MAXARG-1; j++)
        exec_args[exec_args_count++] = argv[j];
      exec_args[exec_args_count] = 0;
      break;
    }

    if(!target_name) target_name = argv[i];
    else {
      fprintf(2, "usage: find <start-path> <name|pattern> [-E | -F] [-exec <cmd> [args...]]\n");
      exit(1);
    }
  }

  if(!target_name) {
    fprintf(2, "usage: find <start-path> <name|pattern> [-E | -F] [-exec <cmd> [args...]]\n");
    exit(1);
  }

  search(start_path, target_name);
  exit(0);
}

