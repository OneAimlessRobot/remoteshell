#!/bin/bash

valgrind --leak-check=full --track-fds=yes ./admin.exe 127.0.0.1 9000 bash
