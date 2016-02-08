#!/usr/bin/env bash
rm -f *.log &
make clean &
clear && clear &
wait
make
echo 'Running...' &
./cachesim -c 12 -b 5 -s 3 -v 3 -C 15 -B 5 -S 4  < traces/astar.trace       2>&1 > astar.log &
./cachesim -c 12 -b 5 -s 0 -v 3 -C 15 -B 5 -S 4  < traces/bzip2.trace       2>&1 > bzip2.log &
./cachesim -c 12 -b 5 -s 3 -v 3 -C 15 -B 5 -S 10 < traces/mcf.trace         2>&1 > mcf.log & 
./cachesim -c 12 -b 5 -s 3 -v 0 -C 15 -B 5 -S 4  < traces/perlbench.trace   2>&1 > perlbench.log &
wait
echo 'Generating report...' &
make clean &
python3 verify.py > verify.log &
wait
