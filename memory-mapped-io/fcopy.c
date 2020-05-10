#include <assert.h>
#include <unistd.h>     // close(), write
#include <stdio.h>
#include <sys/stat.h>   // POSIX header, includes 'stat'

// To compile: clang -o fcopy fcopy.c
int main(int argc, char **argv) {
  struct stat st;
  // Using library calls: buffered I/O
  FILE *src_fptr = fopen(argv[1], "r");
  FILE *dst_fptr = fopen(argv[2], "w");

  assert(src_fptr != NULL && dst_fptr != NULL);
  assert(stat(argv[1], &st) != -1);
  assert(stat(argv[2], &st) != -1);

  const int BUFFER_SIZE = 4096;
  char *buffer[BUFFER_SIZE];

  size_t size  = 0;
  size_t total = 0;
  while ((size = fread(buffer, sizeof(char), BUFFER_SIZE, src_fptr)) != 0) {
    fwrite(buffer, sizeof(char), size, dst_fptr);
    total += size;
  }

  fclose(dst_fptr);
  fclose(src_fptr);

  printf("Total %lu bytes copied\n", total);

  return 0;
}
