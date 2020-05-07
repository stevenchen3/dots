#!/bin/bash

# on Mac OS
dd if=/dev/urandom bs=1024 count=8192     of=8mb_file   conv=notrunc
dd if=/dev/urandom bs=1024 count=32768    of=32mb_file  conv=notrunc
dd if=/dev/urandom bs=1024 count=131072   of=128mb_file conv=notrunc
dd if=/dev/urandom bs=1024 count=524288   of=512mb_file conv=notrunc
dd if=/dev/urandom bs=1024 count=1048576  of=1gb_file   conv=notrunc
dd if=/dev/urandom bs=1024 count=2097152  of=2gb_file   conv=notrunc
dd if=/dev/urandom bs=1024 count=4194304  of=4gb_file   conv=notrunc
dd if=/dev/urandom bs=1024 count=10485760 of=10gb_file  conv=notrunc
