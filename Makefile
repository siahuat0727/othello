CC = g++
CFLAGS += -std=c++11 -Wall

othello: othello.cpp bitboard.h
	$(CC) $(CFLAGS) -o othello othello.cpp

clean:
	rm othello
