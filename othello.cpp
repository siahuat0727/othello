#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include "othello.h"

using namespace std;

#define INF 0x3fffffff
#define NEG_INF 0x80000000
#define BAD 0x80808080
#define GOOD 0x3f3f3f3f


vector<Position> possiblePosition;
typedef struct{
//	Position pos;
	BitBoard board;
	int depth;
	int value;
}Hash;
Hash hashTable[1 << 26]; // save the best position in this 
 
int MY_COLOR;
unsigned long long SEARCH; // TODO delete
unsigned long long HASH_HIT; // TODO delete
unsigned long long COLLISION; // TODO delete

void ai_vs_ai();

int main(){
	ai_vs_ai();
	return 0;
	int color = 0;
	int turn = 20;
	while(turn--){
		whereCanGo(color);
		BOARD.go(possiblePosition[0].x, possiblePosition[0].y, color);
		BOARD.print();
		usleep(3000000);
		color ^= 1;
	}
	return 0;
}


void ai_vs_ai(){
	int no_move = 0;
	int color = 0;
	puts("staring...");
	while(no_move < 2){
		if(!canIGo(color)){
			printf("%d no move...\n", color);
			no_move++;
		}else{
			printf("%d can move at", color);
			printVector(possiblePosition);
			no_move = 0;
			ai_go(color);
		}
		BOARD.print();
		puts("***************************");
		color ^= 1;
	}
	int blackCount = BOARD.countBlack();
	int whiteCount = BOARD.countWhite();
	int winner = BOARD.findWinner(); // TODO do popcount twice
	if(winner == BitBoard::EMPTY)
		puts("due")	;
	else if(winner == BitBoard::BLACK)
		printf("black wins ! %d : %d\n", blackCount, whiteCount);
	else
		printf("white wins ! %d : %d\n", whiteCount, blackCount);
	
	printf("search = %llu\ntotal hash hit = %llu\ncollision = %llu\n", SEARCH, HASH_HIT, COLLISION);
}

int getValue(int color, const BitBoard &board){
	static const bitmap middle        = 0x003c7e7e7e7e3c00;
	static const bitmap edge          = 0x3c0081818181003c;
	static const bitmap star_corner   = 0x0042000000004200; // corner dui jiao
	static const bitmap corner        = 0x8100000000000081;
	static const bitmap around_corner = 0x4281000000008142;

	bitmap my_disks = board.bitDisks[color];
	bitmap op_disks = board.bitDisks[color^1];
	int value = 0;
	value += __builtin_popcountll(my_disks & middle);
	value += __builtin_popcountll(my_disks & edge) << 2;
	value -= __builtin_popcountll(my_disks & star_corner) << 4;
	value -= __builtin_popcountll(my_disks & around_corner) << 2;
	value += __builtin_popcountll(my_disks & corner) << 6;
	
	value -= __builtin_popcountll(op_disks & middle);
	value -= __builtin_popcountll(op_disks & edge) << 2;
	value += __builtin_popcountll(op_disks & star_corner) << 4;
	value += __builtin_popcountll(op_disks & around_corner) << 2;
	value -= __builtin_popcountll(op_disks & corner) << 6;

	/*
	whereCanGo(color, board);
	static const int mobility[8][8] = {
		{5,1,3,3,3,3,1,5},
		{1,1,2,2,2,2,1,1},
		{3,2,2,2,2,2,2,3},
		{3,2,2,2,2,2,2,3},
		{3,2,2,2,2,2,2,3},
		{3,2,2,2,2,2,2,3},
		{1,1,2,2,2,2,1,1},
		{5,1,3,3,3,3,1,5}
	};

	for(Position pos : possiblePosition){
		value += mobility[pos.x][pos.y] << 2; // TODO do at mobility table
	}
	*/

	return value;
}

void printPosValue(const Position &pos,const int &value){
	printf("x = %d\ty = %d\tvalue=%d\n", pos.x, pos.y, value);
}

int alphaBeta(int depth, int color, const BitBoard &prevBoard, int alpha, int beta){
	//printf("in alpha beta depth = %d color = %d\n", depth, color);
	//prevBoard.print();
	SEARCH++;
	int bestValue = NEG_INF;
	if (depth <= 0){
		int value = getValue(color, prevBoard);
		return MY_COLOR == color ? value : -value;
	}

	int boardHash = prevBoard.getHash();
	if(depth <= hashTable[boardHash].depth){
		HASH_HIT++;
		if(prevBoard.isSame(hashTable[boardHash].board)){
			return hashTable[boardHash].value;
		}
		COLLISION++;
		/*
		puts("collision!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		puts("prev");
		prevBoard.print();
		puts("hash");
		hashTable[boardHash].board.print();
		exit(1);
		*/
	}

	if(!canIGo(color, prevBoard)){
		if(!canIGo(color^1, prevBoard)){
			int winner = prevBoard.findWinner();
			if(MY_COLOR == winner)
				return GOOD;
			else if(MY_COLOR == (winner^1))
				return BAD;
			else
				return 0;
		}
		return -alphaBeta(depth-1, color^1, prevBoard, -beta, -alpha);
	}
	vector<Position> possiblePos(possiblePosition);
	BitBoard tmpBoard;
	Position bestPos;
	for(Position pos : possiblePos){
		tmpBoard.copy(prevBoard);
		tmpBoard.go(pos, color);
		int value = -alphaBeta(depth-1, color^1, tmpBoard, -beta, -alpha);
		if(value > alpha){
			if(value >= beta)
				return value;
			alpha = value;
		}
		if(value > bestValue){
			bestValue = value;
			bestPos.copy(pos);
		}
	}
	//	printf("depth = %d\t", depth);
	//	printPosValue(bestPos, bestValue);
	hashTable[boardHash].depth = depth;
	hashTable[boardHash].value = bestValue;
	hashTable[boardHash].board.copy(prevBoard);

	return bestValue;
}

Position minMax(int depth, int color){
	MY_COLOR = color;
	int bestValue = NEG_INF;
	Position bestMove;
	vector<Position> possiblePos(possiblePosition);
	BitBoard tmpBoard;
	for(Position pos : possiblePos){
		tmpBoard.copy(BOARD);
		tmpBoard.go(pos, color);
		int value = -alphaBeta(depth-1, color^1, tmpBoard, 0x80000001, 0x3fff3fff);
		if(value > bestValue){
			bestValue = value;
			bestMove  = pos; // TODO operation= overloading
		}
	}
	printf("best move is %d %d\n", bestMove.x, bestMove.y);
	return bestMove;
}

inline void ai_go(int color){
	BOARD.go(minMax(8, color), color);
}

/*
 * Check where can go and save result at vector possiblePosition
 */
void whereCanGo(int color, BitBoard board){
	possiblePosition.clear();
	for(int x = 0; x < BitBoard::SIZE; ++x)
		for(int y = 0; y < BitBoard::SIZE; ++y){
			if(board.go(x, y, color, true))
				possiblePosition.push_back(Position(x, y));
		}
}

/*
 * Print the vector passed in or vector possiblePosition
 */
void printVector(vector<Position> V = possiblePosition){
	for (Position pos : V)
		printf("(%d, %d), ", pos.x, pos.y);
	putchar('\n');
}

/*
 * Check where can go and save result at vector possiblePosition
 * Return whether can go
 */
inline bool canIGo(int color, const BitBoard &board) {
	whereCanGo(color, board);
	return !possiblePosition.empty();
}
