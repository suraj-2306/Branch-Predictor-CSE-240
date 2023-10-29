CC=g++
OPTS=-g -Werror

all: main.o predictor.o
	$(CC) $(OPTS) -lm -o predictor main.o predictor.o

main.o: main.cpp predictor.h
	$(CC) $(OPTS) -c main.cpp

predictor.o: predictor.h predictor.cpp
	$(CC) $(OPTS) -c predictor.cpp

clean:
	rm -f *.o predictor;
