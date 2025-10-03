CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE

PROGS := ps timeout

all: $(PROGS)

ps: ps.o
timeout: timeout.o

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean
