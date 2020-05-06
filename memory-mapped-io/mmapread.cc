#include <assert.h>
#include <fcntl.h>      // flags: O_RDONLY
#include <unistd.h>     // close(), write
#include <sys/mman.h>   // includes 'mmap', see "man mmap"
#include <sys/stat.h>   // POSIX header, includes 'stat'

//
// To compile: clang++ -std=c++11 mmap-read.cc -o mmap-read
//
int main(int argc, char **argv) {
  // stat is a system struct that is defined to store information about files
  struct stat st;
  stat(argv[1], &st);         // ignore args checks here
  size_t length = st.st_size; // Read the file size

  // Open the given file and have its file descriptor
  int fd = open(argv[1], O_RDONLY, 0);
  assert(fd != -1);

  // Execute the mmap: move the entire content of the given file into memory,
  // if its size bigger than actual physical or available memory, then operating
  // system would use virutal memory (page-in and page-out may happen)
  //
  // nullptr     := address hint, nullptr means no preference on the address and
  //                let the os decide
  // length      := total # of bytes of data load into memory
  // PROT_READ   := protection mode, in this case, read-only
  // MAP_PRIVATE := flags, in this case, modifications are private to the process
  // fd          := file descriptor
  // 0           := offset, 0 means we read from the beginning of the file
  void *ptr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(ptr != MAP_FAILED);

  // Write the mmapped ptr to stdout (= FD #1)
  write(1, ptr, length);

  // Clean up: remove a mapping
  assert(munmap(ptr, length) == 0);

  close(fd); // close the file

  return EXIT_SUCCESS;
}
