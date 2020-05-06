# Memory-mapped File I/O


## Background

Recently, we encountered an issue in one of our live systems that it throws exception regarding
native memory allocation failure from the JVM:

```
OpenJDK 64-Bit Server VM warning: Attempt to protect stack guard pages failed.
#
# There is insufficient memory for the Java Runtime Environment to continue.
# Native memory allocation (mmap) failed to map 12288 bytes for committing reserved memory.
# An error report file with more information is saved as:
#
OpenJDK 64-Bit Server VM warning: Attempt to protect stack guard pages failed.
OpenJDK 64-Bit Server VM warning: Attempt to deallocate stack guard pages failed.
OpenJDK 64-Bit Server VM warning: INFO: os::commit_memory(0x00007f6f7315a000, 12288, 0) failed; error='Cannot allocate memory' (errno=12)
[thread 140116711044864 also had an error]
OpenJDK 64-Bit Server VM warning: INFO: os::commit_memory(0x00007f6f738a1000, 262144, 0) failed; error='Cannot allocate memory' (errno=12)
```

And this is not caused by insufficient heap or direct memory allocation to the Java process. It
turns out the system uses low-level system kernel function (i.e., `mmap`) to achieve memory-mapped file
access to read a large number of files whose size does not fit into memory either. So the operating
system would use virtual memory. However, Linux kernel has a limit (i.e., [vm.max_map_count](
https://www.kernel.org/doc/Documentation/sysctl/vm.txt)) for how many memory map areas a process can
access, which by default is 65535. And in our case, we exceeded this limit. And increasing such
limit would ease the problem. In order to increase this memory map limit, we could use one of the
following two approaches:

### Ephemeral setting using `sysctl` command

If using containers, this needs to be done on the host instead of the containers. The new setting
applies only to newly launched processes or containers. And if the server somehow restarts, this
setting would be gone.

```bash
sudo sysctl -w vm.max_map_count=262144
sudo sysctl -n vm.max_map_count
```

### Permanent setting on the server

The new setting won't take effect until the server restarts, thus we could use above commands to
apply ephemeral setting to avoid server restarts.

```bash
sudo touch /etc/sysctl.d/custom.conf
sudo echo "vm.max_map_count=262144" > /etc/sysctl.d/custom.conf
```

### Use DaemonSet for Kubernetes

Some Kubernetes services won't allow users to change Kubernetes nodes' setting using startup
scripts (e.g., Google Kubernetes Engine). In such case, we could run a DaemonSet to achieve the
goal. See `daemonset.yaml`


```bash
kubectl apply -f daemonset.yaml
```

The side-effect of increasing the `vm.max_map_count` is that processes (or containers) now are
potentially able access more system memory, especially for JVM process that it means JVM process is
able access more than what's allocated (including heap and direct memory). But, whether a process
will do depends on the amount of data it accesses.

However, how to adjust Linux kernel settings is not the primary focus for this discussion. Instead,
I'd like to deep dive into memory-mapped file IO.


## How does computer access I/O devices?

Under the von Neumann model, three major components of a computer system: CPU, main memory and I/O
devices are connected through Bus. And there're two complementary approaches of performing I/O
between CPU and I/O devices: (1) port-mapped I/O and (2) memory-mapped I/O (See [this wikipedia
entry](https://en.wikipedia.org/wiki/Memory-mapped_I/O#Port_I/O_via_device_drivers)).

For port-mapped I/O, CPU has limited set of instructions to interact between CPU registers and I/O
device ports and those instructions are very CPU-specific. Recall that CPU uses instructions like
`LDR` and `STR` to load and store data from and to registers to access memory in its address space.
On the other hand, with memory-mapped I/O, the registers of I/O devices are mapped to memory
addresses in the same address space the CPU uses. They're usually mapped to certain addresses
reserved for I/O devices. When the Bus decodes an address, it knows whether it is an I/O device or
memory. Therefore, CPU can address both memory and I/O devices using the same instructions in the
same address space. For example, when we type a key in our keyboard, the input of such key will go
to the address that is mapped to the key, and later CPU is able to read from that address.


## How does program access files?

Without loss of generality, let's take Linux as an example. Programs running on Linux make system
calls to access files in the file system, e.g., `read()`, `write()`, `mmap()`. These system calls
eventually translate into CPU instructions to access target files on the I/O devices (e.g., disks).

Most programming languages by default call `read()` and `write()` to access files in the file system.
When making a `read()` system call, the operating system first reads bytes from disks to memory in
the kernel space and then copies these bytes to the memory to user space. If all the requested data
are in the page cache, the kernel will copy it over the user space immediately, otherwise, it blocks
the calling thread, arranges the disk to read requested data into page cache. When the requested
data becomes available in the page cache, it resumes calling thread and copies the data.

On the other hand, `mmap()` directly maps memory pages to bytes on disk in the user space. Whenever
there's a page fault for memory-mapped file, the kernel puts the calling thread to sleep and makes
the hard drive seek to the appropriate block and read the data. Without extra copy, `mmap()` should
potentially perform faster than traditional `read()/write()`, however, studies suggest that the
performance of `mmap` is unstable, especially for small files. But, for large files, once the files
are mapped into memory, you can access the entire through an array. Using `read()/write()`, however,
we can only access the file per buffer size each time and iterate for multiple times in order to get
its full content. Arguably, we could set a bigger buffer, but the overhead of the additional copy
between the kernel space and user space can't be neglected in such case.

Both `read()/write()` and `mmap()` defer I/O scheduling, thread scheduling and I/O alignment (all
I/O must be performed in multiples of the storage device's block size, usually 4KB) to the kernel.
In addition, `mmap()` requires memory for maintaining a page table for page eviction. There're other
alternatives to perform file I/O that defers most responsibilities to user. See [this
thread](https://www.scylladb.com/2017/10/05/io-access-methods-scylla/).


## Perform file I/O using `mmap`

In this simple example, we will illustrate how to use `mmap` system call to perform file I/O. See
`mmapread.cc`

```bash
# Check out `mmap()` API doc
man mmap

clang++ -std=c++11 mmapcopy.cc -o mmapcopy
# Copy file
mmapcopy /path/to/source/file /path/to/destination/file
```


# References

- https://www.scylladb.com/2017/10/05/io-access-methods-scylla/
- https://lemire.me/blog/2012/06/26/which-is-fastest-read-fread-ifstream-or-mmap/
