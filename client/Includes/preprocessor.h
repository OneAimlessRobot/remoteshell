#include <ncurses.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "auxFuncs.h"
#include "nodes.h"
#include "comparator.h"
#include "hasher.h"
#include "cmdstruct.h"
#include "doublelist.h"
#include "stackList.h"
#include "dliterator.h"
#include "atomicOps.h"
#include "doublelistcomp.h"
#include "hashtablecomp.h"
#define SERVERLOGSX 50
#define SERVERLOGSY 0


