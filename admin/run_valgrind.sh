#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./admin.exe 192.168.0.100 11090 bash
