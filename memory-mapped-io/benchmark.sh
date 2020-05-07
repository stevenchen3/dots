#!/bin/bash

prefixes="8mb 32mb 128mb 512mb 1gb 2gb 4gb 10gb"
for p in ${prefixes}; do
  echo "--------------------------------"
  echo "mmap copy ${p}"
  rm -f "${p}_file_copy"
  time ./mmapcopy ${p}_file ${p}_file_copy
done

for p in ${prefixes}; do
  echo "--------------------------------"
  echo "fcopy ${p}"
  rm -f "${p}_file_copy"
  time ./fcopy ${p}_file ${p}_file_copy
done
