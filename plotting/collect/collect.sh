#!/bin/bash

# Weak Scaling
N=$((1024 * 64))
file="weak.csv"
if [ -f $file ]
then
        rm $file
fi
echo "P,N,Insertion Speedup, Iteration Speedup, Extraction Speedup" >> $file
for i in $(seq 1 16)
do
        P=$i
        real_N=$(($N * $P))

        cmd="../../bin/evaluate ${real_N}"
        backup=${OMP_NUM_THREADS}
        export OMP_NUM_THREADS=$P;
        echo -n "OMP_NUM_THREADS=${OMP_NUM_THREADS} "
        echo $cmd
        $cmd > tmp
        export OMP_NUM_THREADS=$backup;
        insert_speedup=$(cat tmp | cut -d, -f10)
        iteration_speedup=$(cat tmp | cut -d, -f11)
        extraction_speedup=$(cat tmp | cut -d, -f12)
        rm tmp

        echo "$P,${real_N},${insert_speedup},${iteration_speedup},${extraction_speedup}" >> $file
done

# Strong Scaling
N=$((1024 * 1024))
file="strong.csv"
if [ -f $file ]
then
        rm $file
fi
echo "P,N,Insertion Speedup, Iteration Speedup, Extraction Speedup" >> $file
for i in $(seq 1 16)
do
        P=$i

        cmd="../../bin/evaluate ${N}"
        backup=${OMP_NUM_THREADS}
        export OMP_NUM_THREADS=$P;
        echo -n "OMP_NUM_THREADS=${OMP_NUM_THREADS} "
        echo $cmd
        $cmd > tmp
        export OMP_NUM_THREADS=$backup;
        insert_speedup=$(cat tmp | cut -d, -f10)
        iteration_speedup=$(cat tmp | cut -d, -f11)
        extraction_speedup=$(cat tmp | cut -d, -f12)
        rm tmp

        echo "$P,$N,${insert_speedup},${iteration_speedup},${extraction_speedup}" >> $file
done
