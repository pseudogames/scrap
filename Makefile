
CFLAGS:=-std=c99 -Wall -Werror -ggdb

ifeq ("$(MSYSTEM)","MINGW32")
PREFIX=/d/mingw-w64/x86_64-4.9.0-posix-seh-rt_v3-rev2/mingw64
PATH:=$(PREFIX)/bin:$(PATH)
CFLAGS+=-I$(PREFIX)/include -I/d/lib/SDL2-2.0.3/x86_64-w64-mingw32/include/SDL2 -Dmain=SDL_main
LDFLAGS=-mconsole -static -L/d/mingw-w64/x86_64-4.9.0-posix-seh-rt_v3-rev2/mingw64/x86_64-w64-mingw32/lib -L/d/lib/SDL2-2.0.3/x86_64-w64-mingw32/lib
LIBS=-lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lopengl32 -lpthread
SUFFIX=.exe
endif

ifeq ($(shell echo $$OSTYPE),darwin13)
CFLAGS+=-I/opt/local/include/SDL2 -D_THREAD_SAFE
LDFLAGS:=-L/opt/local/lib -lSDL2 -lgl -lm -liconv -Wl,-framework,OpenGL -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit
endif

CC=gcc
SERVER_OBJS=server.o
CLIENT_OBJS=client.o sdnoise1234.o
TEST_OBJS=test.o sdnoise1234.o
SERVER=rts-px-server$(SUFFIX)
CLIENT=rts-px$(SUFFIX)
TEST=test$(SUFFIX)

.PHONY: all clean system

all: $(TEST) # $(SERVER) $(CLIENT) 

clean:
	rm -fv *.o $(SERVER) $(CLIENT) $(TEST)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(SERVER): $(SERVER_OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(TEST): $(TEST_OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
