#!/usr/bin/bash
 
for f in ./data/midi_examples/*; do

  echo
  echo "DEBUG $f"
  echo "Running hyperfine benchmark"
  hyperfine --warmup 10 "./build/tools/benchmark $f"
  echo

  echo "Running callgrind"
  valgrind --tool=callgrind -q \
           --callgrind-out-file="callgrind.$(basename $f).out" \
          ./build/tools/benchmark $f
  callgrind_annotate "callgrind.$(basename $f).out" | grep "PROGRAM TOTALS"
  echo

  echo "Running massif"
  valgrind --tool=massif -q \
           --pages-as-heap=yes\
           --massif-out-file="massif.$(basename $f).out" \
          ./build/tools/benchmark $f
  mem_use=$(grep mem_heap_B "massif.$(basename $f).out" \
          | sed -e 's/mem_heap_B=\(.*\)/\1/' \
          | sort -g \
          | tail -n 1)
  echo "Total memory use: $mem_use bytes"
  echo


  echo
  echo "RELEASE $f"
  echo "Running hyperfine benchmark"
  hyperfine --warmup 10 "./buildRelease/tools/benchmark $f"
  echo

  echo "Running callgrind"
  valgrind --tool=callgrind -q \
           --callgrind-out-file="callgrind.$(basename $f).out" \
          ./buildRelease/tools/benchmark $f
  callgrind_annotate "callgrind.$(basename $f).out" | grep "PROGRAM TOTALS"
  echo

  echo "Running massif"
  valgrind --tool=massif -q \
           --pages-as-heap=yes\
           --massif-out-file="massif.$(basename $f).out" \
          ./buildRelease/tools/benchmark $f
  mem_use=$(grep mem_heap_B "massif.$(basename $f).out" \
          | sed -e 's/mem_heap_B=\(.*\)/\1/' \
          | sort -g \
          | tail -n 1)
  echo "Total memory use: $mem_use bytes"
  echo

done
