CC = gcc
CFLAGS = -Wall -Iinc

SRCS = src/main.c src/process.c src/memory.c src/sensor.c \
       src/controller.c src/server.c src/random_measurement.c

OBJS = $(SRCS:src/%.c=obj/%.o)
BIN = bin/SOestimate

$(BIN): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ -lm

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o *~

cleanall: clean
	rm -f $(BIN)
	rm -f TMP*