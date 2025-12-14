#!/bin/bash 
#For GNU compiler
#compile commands
g++ -c -o main.o main.cpp -std=c++11
g++ -c -o lru.o lru.cpp -std=c++11
g++ -c -o lfu.o lfu.cpp -std=c++11
g++ -c -o arc.o arc.cpp -std=c++11
g++ -c -o lirs.o lirs.cpp -std=c++11
g++ -c -o cacheus.o cacheus.cpp -std=c++11

#linking command
g++ -std=c++11 -o cache main.o lru.o lfu.o arc.o lirs.o cacheus.o

#hm_0.csv
for policy in LRU LFU LIRS ARC CACHEUS
do
        for csize in 703 3516 7031 35156 70313 140626 281251 562502 632815
        do
                ./cache -m $policy -f 2 -i hm_0.csv -s $csize
        done
done


#hm_1.csv
for policy in LRU LFU LIRS ARC CACHEUS
do
        for csize in 54 272 544 2720 5440 10881 21762 43523 48964
        do
                ./cache -m $policy -f 2 -i hm_1.csv -s $csize
        done
done


#mds_0.csv
for policy in LRU LFU LIRS ARC CACHEUS
do
        for csize in 703 3514 7028 35142 70284 140568 281137 562274 632558
        do
                ./cache -m $policy -f 2 -i mds_0.csv -s $csize
        done
done

#mds_1.csv
for policy in LRU LFU LIRS ARC CACHEUS
do
        for csize in 703 3514 7028 35142 70284 140568 281137 562274 632558
        do
                ./cache -m $policy -f 2 -i mds_1.csv -s $csize
        done
done

#proj_3.csv
for policy in LRU LFU LIRS ARC CACHEUS
do
        for csize in 1463 7317 14633 73167 146334 292667 585334 1170668 1317002
        do
                ./cache -m $policy -f 2 -i proj_3.csv -s $csize
        done
done