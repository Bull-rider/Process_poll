SOURCES:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES))
ELF:=client
CC:=gcc
CFLAGS:=-Wall
$(ELF):$(OBJS)
	gcc $^ -o $@
clean:
	rm -rf $(OBJS) $(ELF)
