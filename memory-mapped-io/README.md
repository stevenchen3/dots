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
goal.


```yaml
kind: DaemonSet
apiVersion: extensions/v1beta1
metadata:
  name: startup-scripts
  labels:
    app: startup-scripts
spec:
  template:
    metadata:
      labels:
        app: startup-scripts
    spec:
      hostPID: true
      containers:
      - name: startup-scripts
        image: gcr.io/google-containers/startup-script:v1
        imagePullPolicy: Always
        securityContext:
          privileged: true
        resources:
          requests:
            memory: 16Mi
            cpu: 16m
          limits:
            memory: 16Mi
            cpu: 16m
        env:
        - name: STARTUP_SCRIPT
          value: |
            #! /bin/bash
            sudo sysctl -w vm.max_map_count=262144
            sudo sysctl -n vm.max_map_count
            sudo touch /etc/sysctl.d/custom.conf
            sudo echo "vm.max_map_count=262144" > /etc/sysctl.d/custom.conf
```

However, how to adjust Linux kernel settings is not the primary focus for this discussion. Instead,
I'd like to deep dive into memory-mapped file IO.


## How computer accesses I/O devices

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

## How does file read and write work?
