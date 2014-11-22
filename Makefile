
CFLAGS:=-std=c99 -Wall -Werror -ggdb -O0

SERVER=rts-px-server
CLIENT=rts-px
TEST=test

ifeq ("$(MSYSTEM)","MINGW32")
CFLAGS+=-I./include -Dmain=SDL_main
LDFLAGS=-L./lib -mconsole -static
LIBS=-lmingw32 -lSDL2main -lSDL2 -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -lglew32 -lglu32 -lopengl32 -lpthread
SUFFIX=.exe

$(SERVER): $(SERVER)$(SUFFIX)
$(CLIENT): $(CLIENT)$(SUFFIX)
$(TEST): $(TEST)$(SUFFIX)
endif

ifeq ($(shell echo $$OSTYPE),darwin13)
CFLAGS+=-I/opt/local/include/SDL2 -D_THREAD_SAFE
LDFLAGS:=-L/opt/local/lib -lSDL2 -lgl -lm -liconv -Wl,-framework,OpenGL -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit
endif

CC=gcc
SERVER_OBJS=server.o
CLIENT_OBJS=client.o sdnoise1234.o
TEST_OBJS=test.o display.o array.o sdnoise1234.o log.o 

.PHONY: all clean system

all: $(SERVER)$(SUFFIX) $(CLIENT)$(SUFFIX) $(TEST)$(SUFFIX)

DEPS_H_SRCS := $(wildcard *.h)
DEPS_C_SRCS := $(wildcard *.c)
DEPS = .deps

$(DEPS): $(DEPS_C_SRCS) $(DEPS_H_SRCS)
	$(CC) $(CFLAGS) -MM $(DEPS_C_SRCS) > $@

-include $(DEPS)

clean:
	rm -fv *.o $(SERVER)$(SUFFIX) $(CLIENT)$(SUFFIX) $(TEST)$(SUFFIX) $(DEPS)

$(SERVER)$(SUFFIX): $(SERVER_OBJS)
$(CLIENT)$(SUFFIX): $(CLIENT_OBJS)
$(TEST)$(SUFFIX): $(TEST_OBJS)
$(SERVER)$(SUFFIX) $(CLIENT)$(SUFFIX) $(TEST)$(SUFFIX):
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@


