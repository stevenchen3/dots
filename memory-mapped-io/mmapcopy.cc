#include <assert.h>
#include <fcntl.h>      // flags: O_RDONLY
#include <unistd.h>     // close(), write
#include <string.h>
#include <sys/mman.h>   // includes 'mmap', see "man mmap"
#include <sys/stat.h>   // POSIX header, includes 'stat'

//
// To compile: clang++ -std=c++11 mmapcopy.cc -o mmapcopy
//
int main(int argc, char **argv) {
  // stat is a system struct that is defined to store information about files
  struct stat st;
  stat(argv[1], &st);         // ignore args checks here and below
  size_t length = st.st_size; // Read the file size

  // Open the given source file and have its file descriptor
  int src_fd = open(argv[1], O_RDONLY, 0);
  int dst_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0666);
  assert(src_fd != -1 && dst_fd != -1);

  // Set the size of the newly create file the same as the source file
  ftruncate(dst_fd, length);

  // Execute the mmap: move the entire content of the given file into memory,
  // if its size bigger than actual physical or available memory, then operating
  // system would use virutal memory (page-in and page-out may happen)
  //
  // nullptr     := address hint, nullptr means no preference on the address and
  //                let the os decide
  // length      := total # of bytes of data load into memory
  // PROT_READ   := protection mode, in this case, read-only
  // MAP_PRIVATE := flags, in this case, modifications are private to the process
  // src_fd      := file descriptor
  // 0           := offset, 0 means we read from the beginning of the file
  void *src_ptr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, src_fd, 0);
  void *dst_ptr = mmap(nullptr, length, PROT_WRITE, MAP_SHARED, dst_fd, 0);
  assert(src_ptr != MAP_FAILED && dst_ptr != MAP_FAILED);

  memcpy(dst_ptr, src_ptr, length);

  // Clean up: remove a mapping
  assert(munmap(src_ptr, length) == 0);
  assert(munmap(dst_ptr, length) == 0);

  close(src_fd); // close the file
  close(dst_fd); // close the file

  return EXIT_SUCCESS;
}
