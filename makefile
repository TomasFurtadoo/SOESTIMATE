CC = gcc
CFLAGS = -Wall -Iinc

SRCS = src/main.c src/process.c src/memory.c src/sensor.c \
       src/controller.c src/server.c src/random_measurement.c src/synchronization.c src/ctime.c src/cstats.c src/csignal.c src/csettings.c src/clog.c

OBJS = $(SRCS:src/%.c=obj/%.o)
BIN = bin/SOestimate

$(BIN): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lm -lpthread -lrt

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o *~
	rm -f log.txt stats.txt

cleanall: clean
	rm -f $(BIN)
	rm -f TMP*