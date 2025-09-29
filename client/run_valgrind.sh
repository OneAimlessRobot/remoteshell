#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./client.exe 9000 192.168.1.2
