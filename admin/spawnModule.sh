#!/bin/bash

if [ $# -ne 1 ]; 
then
	echo "Nao tens argumentos eiiaaadizer"
	exit -1
fi

touch "Includes/$1.h" "Sources/$1.c"
./head* "Includes/$1.h"
