CC = gcc
CFLAGS = -Wall -Iinc

SRCS = src/main.c src/process.c src/memory.c src/sensor.c src/controller.c src/server.c
OBJS = $(SRCS:src/%.c=obj/%.o)

bin/SOestimate: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)/*.o *~

cleanall: clean
	rm -f $(BIN)/SOestimate
	rm -f TMP*