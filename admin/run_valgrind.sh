#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./admin.exe 0.0.0.0 13000 bash 1 80 80
