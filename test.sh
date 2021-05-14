#!/bin/bash

count=0 # number of test cases run so far

# Note that the longtimetable test may take a few seconds to complete.
echo "---Generating binary files from the .asm files---"
for test in tests/*.asm; do
    # Create the corresponding .x2017 file first using the ascii to binary converter
    ./ascii_to_binary $test
done
echo ""
echo "---Running tests---"
for test in tests/*.asm; do
    name=$(basename $test .asm)
    expected=tests/$name.out
    binary=tests/$name.x2017
    echo "Running test $name"
    ./vm_x2017 $binary | diff - $expected || echo "Test $name: failed!"
    count=$((count+1))
done

echo "Finished running $count tests!"