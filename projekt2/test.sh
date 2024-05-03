#!/bin/bash

# kontrola argumentov
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 input_file steps"
    exit 1
fi

# ocakava sa ze vstupny subor bude mat 1 prazdny riadok nakonci
input_file=$1
steps=$2
 
# na zaklade poctu riadkov sa spusti prislusny pocet procesov
proc=$(($(wc -l < "$1")))

input_data=$(<"$input_file")

mpic++ --prefix /usr/local/share/OpenMPI -o life life.cpp
mpirun --prefix /usr/local/share/OpenMPI -np $proc life $input_data $steps
rm -f life