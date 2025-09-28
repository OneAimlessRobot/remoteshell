
LDLIBS=  -lm -lncurses -lpthread -lutil

XTRAFUN_INCLUDE= ./xtrafun/Includes
XTRAFUN_SOURCES= ./xtrafun/Sources
XTRAFUN_RESDIR= ./xtrafun/resources

CLIENT_INCLUDE= ./client/Includes
CLIENT_SOURCES= ./client/Sources ./client
CLIENT_RESDIR= ./clientresources
CLIENT_BINARY= ./client/client.exe


SERVER_INCLUDE= ./admin/Includes
SERVER_SOURCES= ./admin/Sources ./admin
SERVER_RESDIR= ./admin/resources

SERVER_BINARY= ./admin/admin.exe

CURRDIR=echo `pwd`

CC= clang
DEPFLAGS= -MP -MD

CFLAGS= -fsanitize=thread -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -gdwarf-4 $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)
#CFLAGS= -fsanitize=address -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -gdwarf-4 $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)
#CFLAGS= -Wall -DPROGRAMPATHAUX="$(CURRDIR)"  -Wextra -gdwarf-4 $(foreach D, $(INCLUDE), -I$(D)) $(DEPFLAGS)

SERVER_SOURCEFILES=$(foreach D,$(SERVER_SOURCES), $(wildcard $(D)/*.c))

CLIENT_SOURCEFILES=$(foreach D,$(CLIENT_SOURCES), $(wildcard $(D)/*.c))

XTRAFUN_SOURCEFILES=$(foreach D,$(XTRAFUN_SOURCES), $(wildcard $(D)/*.c))


XTRAFUN_RESOURCEFILES=$(wildcard $(XTRAFUN_RESDIR)/*.o)

CLIENT_RESOURCEFILES=$(wildcard $(CLIENT_RESDIR)/*.o)

SERVER_RESOURCEFILES=$(wildcard $(SERVER_RESDIR)/*.o)


XTRAFUN_OBJECTS=$(patsubst %.c,%.o,$(XTRAFUN_SOURCEFILES))


CLIENT_OBJECTS=$(patsubst %.c,%.o,$(CLIENT_SOURCEFILES))


SERVER_OBJECTS=$(patsubst %.c,%.o,$(SERVER_SOURCEFILES))

ALLMODULES=  $(XTRAFUN_RESOURCEFILES) $(XTRAFUN_OBJECTS) $(SERVER_RESOURCEFILES) $(SERVER_OBJECTS) $(CLIENT_RESOURCEFILES) $(CLIENT_OBJECTS)

CLIENT_DEPFILES= $(patsubst %.c,%.d,$(CLIENT_SOURCEFILES))

SERVER_DEPFILES= $(patsubst %.c,%.d,$(SERVER_SOURCEFILES))

XTRAFUN_DEPFILES= $(patsubst %.c,%.d,$(XTRAFUN_SOURCEFILES))


all: $(server) $(client)

server: $(SERVER_BINARY)
	echo $(LDLIBS)
	echo $(CURRDIR)

client: $(CLIENT_BINARY)
	echo $(LDLIBS)
	echo $(CURRDIR)


$(SERVER_BINARY): $(SERVER_OBJECTS)  $(XTRAFUN_OBJECTS)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(SERVER_RESOURCEFILES)  $(LDLIBS)

$(CLIENT_BINARY): $(SERVER_OBJECTS) $(XTRAFUN_OBJECTS)
	$(CC) -g -v  $(CFLAGS) -o  $@ $^ $(SERVER_RESOURCEFILES)  $(LDLIBS)


%.o:%.c
	$(CC) -g  $(CFLAGS) -c -o $@ $<
	echo $(CFLAGS)

clean_all:
	rm -rf $(BINARY) $(ALLMODULES) $(DEPFILES)
