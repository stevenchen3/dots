#include <assert.h>
#include <fcntl.h>      // flags: O_RDONLY
#include <unistd.h>     // close(), write
#include <stdio.h>

// To compile: clang -o copy copy.c
int main(int argc, char **argv) {
  // Using system calls
  int src_fd = open(argv[1], O_RDONLY, 0);
  int dst_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0666);
  assert(src_fd != -1 && dst_fd != -1);

  const int BUFFER_SIZE = 4096;
  char *buffer[BUFFER_SIZE];

  size_t size  = 0;
  size_t total = 0;
  while ((size = read(src_fd, buffer, BUFFER_SIZE)) != 0) {
    write(dst_fd, buffer, size);
    total += size;
  }

  close(dst_fd);
  close(src_fd);

  printf("Total %lu bytes copied\n", total);

  return 0;
}
