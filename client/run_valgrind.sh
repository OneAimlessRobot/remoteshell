#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./client.exe 192.168.0.100 13000 1
