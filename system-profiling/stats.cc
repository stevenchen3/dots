#include <iostream>
#include <filesystem>    // '-std=c++17'

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/sysctl.h>

namespace fs = std::filesystem;

// https://opensource.apple.com/source/xnu/xnu-201.5/bsd/sys/sysctl.h.auto.html
// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/sysctl.3.html
void get_counter(const char *name, size_t len, long &count) {
  int *mib;
  mib = new int[len];
  sysctlnametomib(name, mib, &len);
  size_t size = sizeof(count);
  if(sysctl(mib, len, &count, &size, nullptr, 0) == -1) {
    count = -1;
  }
}

int main() {
  long maxproc   = 0;
  long ncpu      = 0;
  long physmem   = 0;
  long usermem   = 0;
  long byteorder = 0;
  long pagesize  = 0;
  get_counter("kern.maxproc", 2, maxproc);
  get_counter("hw.ncpu", 2, ncpu);
  get_counter("hw.physmem", 2, physmem);
  get_counter("hw.usermem", 2, usermem);
  get_counter("hw.byteorder", 2, byteorder);
  get_counter("hw.pagesize", 2, pagesize);

  printf("-------- kernel and hardware info --------\n");
  printf("Max %ld concurrent process supported\n", maxproc);
  printf("Total %ld CPUs installed\n", ncpu);
  printf("Total %ld bytes of physical memory\n", physmem);
  printf("Total %ld bytes of non-kernel memory\n", usermem);
  printf("The byteorder (little endian: 4321 or big endian: 1234) %ld\n", byteorder);
  printf("The software page size is %ld bytes\n", pagesize);

  // Get file system information
  const char *path = "/"; // any path under the mount point would work
  struct statvfs buf;
  statvfs(path, &buf);
  printf("-------- file system info --------\n");
  printf("file system block size: %ld\n", buf.f_bsize);
  printf("fragment size: %ld\n", buf.f_frsize);
  printf("Size of fs in f_frsize units: %d\n", buf.f_blocks);
  printf("file system ID: %ld\n", buf.f_fsid);
  printf("Number of free blocks: %d\n", buf.f_bfree);
  printf("Number of free blocks for unprivileged users: %d\n", buf.f_bavail);
  printf("Number of inodes (file): %d\n", buf.f_files);
  printf("Number of free inodes: %d\n", buf.f_ffree);
  printf("Number of free inodes for unprivileged users: %d\n", buf.f_favail);
  printf("mount flags: %ld\n", buf.f_flag);
  printf("Maximum filename length: %ld\n", buf.f_namemax);

  // Get the space info of specific path
  fs::space_info tmp = fs::space("/tmp");
  std::cout << "Free space: " << tmp.free << '\n'
            << "Available space: " << tmp.available << '\n';
  return 0;
}
