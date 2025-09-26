#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./admin.exe 192.168.1.4 9000 bash 1
