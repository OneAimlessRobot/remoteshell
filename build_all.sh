#!/bin/bash

make clean_all && make server -j 3 && make client -j 3
