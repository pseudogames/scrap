ifdef __MINGW32__
SYS=MINGW32

PREFIX=/d/mingw-w64/x86_64-4.9.0-posix-seh-rt_v3-rev2/mingw64

PATH:=$(PREFIX)/bin:$(PATH)
CC=gcc
CFLAGS=-Wall -Werror -I$(PREFIX)/include -I/d/lib/SDL2-2.0.3/x86_64-w64-mingw32/include/SDL2 -Dmain=SDL_main
LDFLAGS=-static -L/d/lib/SDL2-2.0.3/x86_64-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -Xlinker
LDFLAGS_SERVER=-mconsole
LDFLAGS_CLIENT=-mconsole # -mwindows
endif

ifeq ($(shell echo $$OSTYPE),darwin13)
SYS=APPLE

PREFIX=

CC=gcc
CFLAGS:=-Wall -Werror -I/opt/local/include/SDL2 -D_THREAD_SAFE
LDFLAGS:=-L/opt/local/lib -lSDL2 -lm -liconv -Wl,-framework,OpenGL -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit
LDFLAGS_SERVER=
LDFLAGS_CLIENT=
endif

SERVER_OBJS=server.o
CLIENT_OBJS=client.o sdnoise1234.o
SERVER=rts-px-server
CLIENT=rts-px

.PHONY: all clean system

all: $(SERVER) $(CLIENT)

clean:
	rm -fv *.o $(SERVER) $(CLIENT)

system:
	echo $(SYS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS_CLIENT) $(LDFLAGS) $^ -o $@

$(SERVER): $(SERVER_OBJS)
	$(CC) $(LDFLAGS_SERVER) $(LDFLAGS) $^ -o $@
