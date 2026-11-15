#!/bin/sh
set -eu
make
workdir=$(mktemp -d)
trap 'rm -rf "$workdir"' EXIT HUP INT TERM
./campuscompute schedule compare jobs.txt > "$workdir/scheduling"
cat "$workdir/scheduling"
grep -q 'Average turnaround: 6.33 | Average waiting: 3.33' "$workdir/scheduling"
grep -q 'Average turnaround: 5.67 | Average waiting: 2.67' "$workdir/scheduling"
./campuscompute banker safe.txt > "$workdir/safe"
cat "$workdir/safe"
grep -q '^SAFE. Safe sequence:' "$workdir/safe"
./campuscompute banker unsafe.txt > "$workdir/unsafe"
cat "$workdir/unsafe"
grep -q '^UNSAFE:' "$workdir/unsafe"
./campuscompute logger
./campuscompute ipc
./campuscompute worker normal
./campuscompute worker fail
./campuscompute limit
if ./campuscompute nonsense; then echo 'FAIL: invalid command accepted'; exit 1; fi
if ./campuscompute schedule rr nonexistent_file; then echo 'FAIL: missing input accepted'; exit 1; fi
echo 'All module checks completed.'
