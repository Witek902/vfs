#!/bin/bash

echo "FRAGMENTATION ========================================="

FS=small1.bin
./vmkfs 100000 $FS
./vcp $FS --up small.txt 0.txt
./vcp $FS --up small.txt 1.txt
./vcp $FS --up small.txt 2.txt
./vcp $FS --up small.txt 3.txt
./vcp $FS --up small.txt 4.txt
./vcp $FS --up small.txt 5.txt
./vcp $FS --up small.txt 6.txt
./vcp $FS --up small.txt 7.txt
./vcp $FS --up small.txt 8.txt
./vcp $FS --up small.txt 9.txt
./vcp $FS --up small.txt 10.txt
./vcp $FS --up small.txt 11.txt
./vcp $FS --up small.txt 12.txt
./vcp $FS --up small.txt 13.txt
./vcp $FS --up small.txt 14.txt
./vrm $FS 1.txt
./vrm $FS 3.txt
./vrm $FS 5.txt
./vrm $FS 7.txt
./vrm $FS 9.txt
./vcp $FS --up big.txt big.txt
./vls $FS

echo
echo "NO FRAGMENTATION ========================================="

FS=small2.bin
./vmkfs 100000 $FS
./vcp $FS --up small.txt 0.txt
./vcp $FS --up small.txt 2.txt
./vcp $FS --up small.txt 4.txt
./vcp $FS --up small.txt 6.txt
./vcp $FS --up small.txt 8.txt
./vcp $FS --up small.txt 10.txt
./vcp $FS --up small.txt 11.txt
./vcp $FS --up small.txt 12.txt
./vcp $FS --up small.txt 13.txt
./vcp $FS --up small.txt 14.txt
./vcp $FS --up big.txt big.txt
./vls $FS


