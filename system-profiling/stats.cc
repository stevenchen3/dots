#include <iostream>

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>

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

  printf("Max %ld concurrent process supported\n", maxproc);
  printf("Total %ld CPUs installed\n", ncpu);
  printf("Total %ld bytes of physical memory\n", physmem);
  printf("Total %ld bytes of non-kernel memory\n", usermem);
  printf("The byteorder (little endian: 4321 or big endian: 1234) %ld\n", byteorder);
  printf("The software page size is %ld bytes\n", pagesize);
  return 0;
}
