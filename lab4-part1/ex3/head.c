#include "types.h"
#include "stat.h"
#include "user.h"

char buf[512];

void
head(int fd)
{
  int n;
  int lines = 0;

  while((n = read(fd, buf, sizeof(buf))) > 0 && lines < 5) {
    for(int i = 0; i < n; i++) {
      if(buf[i] == '\n')
        lines ++;
      if(lines == 5) {
        n = i + 1;
        buf[n] = '\0';
        break;
      }
    }
    if (write(1, buf, n) != n) {
      printf(1, "head: write error\n");
      exit();
    }
  }
  if(n < 0){
    printf(1, "head: read error\n");
    exit();
  }
}

int
main(int argc, char *argv[])
{
  int fd, i;

  if(argc <= 1){
    head(0);
    exit();
  }

  for(i = 1; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "head: cannot open %s\n", argv[i]);
      exit();
    }
    head(fd);
    close(fd);
  }
  exit();
}
