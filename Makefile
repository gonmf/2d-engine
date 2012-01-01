all:	
	$(CC) -std=c99 -O3 -lX11 -lrt -lm -Wall -o ./runnable \
	sprite.c \
	engine.c \
	clock.c \
	entry.c
	
run: ./runnable
	./runnable

clean: ./runnable
	$(RM) ./runnable
