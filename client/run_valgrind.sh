#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./client.exe 192.168.1.4 9000 1
