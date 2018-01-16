#ifndef BITBOARD_CPP
#define BITBOARD_CPP

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <assert.h>

using namespace std;

typedef unsigned long long bitmap;

struct Position {
	short x, y;
	Position():x(-1),y(-1) {}
	Position(const int x, const int y):x((short)x),y((short)y) {}
	Position(const Position &pos):x(pos.x),y(pos.y){}
	void copy(const Position &pos){
		x = pos.x;
		y = pos.y;
	}
};

class BitBoard {
	public:
		static const int BLACK = 0;
		static const int WHITE = 1;
		static const int EMPTY = 2;
		static const int SIZE = 8;
		static const int DIRECTION[8][2];

		bitmap bitDisks[2];

		BitBoard();

		bool isBlack(int x, int y) const;
		bool isWhite(int x, int y) const;
		int  isWhat (int x, int y) const;
		bool setBit(int x, int y, int color);
		bool testBit(int x, int y, int color) const;
		bool testBit(int x, int y, bitmap b) const;

		int findWinner() const;
		int countBlack() const;
		int countWhite() const;

		unsigned int getHash() const;

		void flip(int x, int y);
		bool go(Position pos, int color, bool checkOnly=false) ;
		bool go(int x, int y, int color, bool checkOnly=false);
		void print() const;
		void copy(const BitBoard &board);
		bool isSame(const BitBoard &board) const;
};

const int BitBoard::DIRECTION[8][2] = {{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0}};

BitBoard::BitBoard()
{
	bitDisks[BLACK] = 0;
	bitDisks[WHITE] = 0;
	setBit(3, 4, BLACK);
	setBit(4, 3, BLACK);
	setBit(3, 3, WHITE);
	setBit(4, 4, WHITE);
}

inline bool BitBoard::testBit(int x, int y, bitmap b) const {
	return (b >> (x<<3) >> y) & 1; // (b >> 8*x+y) & 1
}

inline bool BitBoard::isBlack(int x, int y) const {
	return testBit(x, y, bitDisks[BLACK]);
}

inline bool BitBoard::isWhite(int x, int y) const {
	return testBit(x, y, bitDisks[WHITE]);
}

inline int BitBoard::isWhat(int x, int y) const {
	return isBlack(x, y) ? BLACK : isWhite(x, y) ? WHITE : EMPTY;
}

inline void BitBoard::flip(int x, int y) {
	bitDisks[0] ^= 1ULL << (x<<3) << y;
	bitDisks[1] ^= 1ULL << (x<<3) << y;
}

inline bool BitBoard::setBit(int x, int y, int color) {
	return bitDisks[color] |= 1ULL << (x<<3) << y;
}

inline bool BitBoard::testBit(int x, int y, int color) const {
	return testBit(x, y, bitDisks[color]);
}

void BitBoard::print() const{
	putchar('\n');
	putchar(' ');
	for(int i = 0; i < SIZE; ++i)
		printf(" %c", i+'A');
	putchar('\n');
	for(int i = 0; i < SIZE; ++i) {
		printf("%d", i+1);
		for(int j = 0; j < SIZE; ++j) {
			printf(" %c", isBlack(i,j) ? 'o' : isWhite(i,j) ? 'x' : ' ');
		}
		putchar('\n');
	}
	printf("   o : black %d\tx : white %d\n", countBlack(), countWhite());
}

inline void BitBoard::copy(const BitBoard &board){
	bitDisks[0] = board.bitDisks[0];
	bitDisks[1] = board.bitDisks[1];
}

inline bool BitBoard::go(Position pos, int color, bool checkOnly) {
	return go(pos.x, pos.y, color, checkOnly);
}

inline int BitBoard::countBlack() const{ // TODO delete
	return __builtin_popcountll(bitDisks[0]);
} 

inline int BitBoard::countWhite() const{
	return __builtin_popcountll(bitDisks[1]);
} 

unsigned int BitBoard::getHash() const {
	bitmap rotatedWhite = bitDisks[1];
	rotatedWhite = (rotatedWhite & 0xf0f0f0f000000000) >> 4  | (rotatedWhite & 0x0f0f0f0f00000000) >> 32 | (rotatedWhite & 0x00000000f0f0f0f0) << 32 | (rotatedWhite & 0x000000000f0f0f0f) << 4;
	rotatedWhite = (rotatedWhite & 0xcccc0000cccc0000) >> 2  | (rotatedWhite & 0x3333000033330000) >> 16 | (rotatedWhite & 0x0000cccc0000cccc) << 16 | (rotatedWhite & 0x0000333300003333) << 2;
	rotatedWhite = (rotatedWhite & 0xaa00aa00aa00aa00) >> 1  | (rotatedWhite & 0x5500550055005500) >> 8 | (rotatedWhite & 0x00aa00aa00aa00aa) << 8  | (rotatedWhite & 0x0055005500550055) << 1;
	unsigned int hash32 = (unsigned int)(((bitDisks[0]>>32) ^ bitDisks[0]) ^ ((rotatedWhite>>32) ^ rotatedWhite));
	unsigned int hash26 = (hash32 >> 6) ^ (hash32 & 0x03ffffff);
	assert((hash26 & 0xfc000000) == 0); // TODO check and inline
	/*
	print();
	printf("hash value = %u\n", hash26);
	*/
	return hash26;
}

inline int BitBoard::findWinner() const {
	int blackCount = __builtin_popcountll(bitDisks[0]);
	int whiteCount = __builtin_popcountll(bitDisks[1]);
	return blackCount > whiteCount ? BLACK : blackCount < whiteCount ? WHITE : EMPTY; // EMPTY means due
}

inline bool BitBoard::isSame(const BitBoard &board) const {
	return bitDisks[0] == board.bitDisks[0] && bitDisks[1] == board.bitDisks[1];
}

bool BitBoard::go(int x, int y, int color, bool checkOnly) {
	if (x < 0)
		return false; // (-1,-1) means can't move TODO check if need

	if (isWhat(x, y) != EMPTY)
		return false;

	const int opinionColor = color ^ 1;

	bool canMove = false;
	static vector<Position> posToReverse;
	for (int dir = 0; dir < 8; dir++) {
		posToReverse.clear();
		int x_t = x;
		int y_t = y;
		while (true) {
			x_t += DIRECTION[dir][0];
			y_t += DIRECTION[dir][1];
			if (x_t < 0 || x_t >= SIZE || y_t < 0 || y_t >= SIZE) {
				posToReverse.clear();
				break;
			}
			int disk = isWhat(x_t, y_t);
			if (disk == opinionColor)
				posToReverse.push_back(Position(x_t, y_t));
			else if (disk == EMPTY) {
				posToReverse.clear();
				break;
			} else
				break;
		}
		if (posToReverse.size()) {
			canMove = true;
			if (checkOnly)
				return true;
			for (Position pos : posToReverse)
				flip(pos.x, pos.y);
		}
	}
	if (canMove) {
		setBit(x, y, color);
		return true;
	}
	return false;
}

#endif
