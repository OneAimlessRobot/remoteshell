#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./client.exe 9000 127.0.0.1
