#include <errno.h>
#include <fcntl.h>
/* #include <sys/stat.h> don't include lstat otherwise the wrong version is hooked */
#include <stdlib.h>
#include "ignore_stub.h"
#include <stdbool.h>

bool ignore(const char* p) {
  return ignore_hs((char*)p);
}

struct stat;

int lstat(const char*, struct stat*);

// from http://opensource.apple.com/source/dyld/dyld-210.2.3/include/mach-o/dyld-interposing.h
#define DYLD_INTERPOSE(_replacement,_replacee) \
   __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
            __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

int(*lstat_old)(const char*, struct stat*);
int lstat_new(const char* path, struct stat* buf) {
  if(ignore(path)) {
    errno = ENOENT;
    return -1;
  }
  return lstat(path, buf);
}
DYLD_INTERPOSE(lstat_new, lstat);

int open_new(const char* path, int flag, int mode) {
  if(ignore(path)) {
    errno = ENOENT;
    return -1;
  }
  return open(path, flag, mode);
}
DYLD_INTERPOSE(open_new, open);

__attribute__((__constructor__)) static void initialize(void) {
  int* argc = malloc(sizeof(int));
  *argc = 0;
  hs_init(argc, 0);
  // TODO: should happen when unloaded
  /* hs_exit(); */
}
