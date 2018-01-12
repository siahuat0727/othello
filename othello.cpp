#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>
#include "othello.h"

using namespace std;

#define INF 0x3fffffff
#define NEG_INF 0x80000000
#define BAD 0x80808080
#define GOOD 0x3f3f3f3f

#define HASH

vector<Position> possiblePosition;
typedef struct{
	BitBoard board;
	int depth;
	int value;
}Hash;
Hash hashTable[1 << 26]; // save the best position in this 
 
int MY_COLOR;
unsigned long long SEARCH; // TODO delete
unsigned long long HASH_HIT; // TODO delete
unsigned long long COLLISION; // TODO delete

int maiN(){
	int my_color = -1;
	while(my_color>>1){
		puts("please input my color");
		scanf("%d", &my_color);
	}
	hm_vs_ai(my_color, BOARD, 2);
	return 0;
}

int main(){
	printf("sizeof hash = %d\n", sizeof(Hash));
	unsigned int start = get_time();
	ai_vs_ai(8);
	unsigned int end = get_time();
	printf("time used: %u ms\n", end-start);
	return 0;
}

void hm_vs_ai(const int ai_color, BitBoard &board, int depth){
	int no_move = 0;
	const int hm_color = ai_color ^ 1;
	int color = BitBoard::BLACK;
	while(no_move < 2){
		if (color == ai_color){
			if(!canIGo(ai_color)){
				puts("I have no move...");
				no_move++;
			}else{
				no_move = 0;
				ai_go(ai_color, depth);
				board.print();
			}
		}else if(color == hm_color){
			if(!canIGo(hm_color)){
				no_move++;
				printf("you have no move...\n");
			}else{
				printf("please input your move (e.g.  2 3): ");
				char buf[10] = "";
				Position pos;
				int depth = 0;
				while(fgets(buf, 9, stdin)!= NULL){
					if(sscanf(buf, "%hd %hd", &pos.x, &pos.y) == 2){
						if(board.go(pos, hm_color) == false){
							printf("Invalid move %d %d, please select move again\n", pos.x, pos.y);
							continue;
						}
						printf("success! received x = %hd, y = %hd\n", pos.x, pos.y);
						board.print();
						break;
					}else if(sscanf(buf, "d%d", &depth) > 0){
						const int max_depth = 13;
						if(depth <= 0){
							depth = 5;
							printf("set default depth = 5\n");
						}else if(depth > max_depth){
							depth = max_depth;
							printf("too deep, set to %d\n", max_depth);
						}
					}
				}
			}
		}else{
			fprintf(stderr, "sth wrong, color = %d\n", color);
			exit(EXIT_FAILURE);
		}
		color ^= 1;
	}
	int blackCount = board.countBlack();
	int whiteCount = board.countWhite();
	int winner = board.findWinner(); // TODO do popcount twice
	if(winner == BitBoard::EMPTY)
		puts("due")	;
	else if(winner == BitBoard::BLACK)
		printf("black wins ! %d : %d\n", blackCount, whiteCount);
	else
		printf("white wins ! %d : %d\n", whiteCount, blackCount);
	
	printf("search = %llu\ntotal hash hit = %llu\ncollision = %llu\n", SEARCH, HASH_HIT, COLLISION);
}

void ai_vs_ai(int depth){
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
			ai_go(color, depth);
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
	value += __builtin_popcountll(my_disks & corner) << 10;
	
	value -= __builtin_popcountll(op_disks & middle);
	value -= __builtin_popcountll(op_disks & edge) << 2;
	value += __builtin_popcountll(op_disks & star_corner) << 4;
	value += __builtin_popcountll(op_disks & around_corner) << 2;
	value -= __builtin_popcountll(op_disks & corner) << 10;

	whereCanGo(color, board);
	static const int mobility[8][8] = {
		{20, 4,12,12,12,12, 4,20},
		{ 4, 4, 8, 8, 8, 8, 4, 4},
		{12, 8, 8, 8, 8, 8, 8,12},
		{12, 8, 8, 8, 8, 8, 8,12},
		{12, 8, 8, 8, 8, 8, 8,12},
		{12, 8, 8, 8, 8, 8, 8,12},
		{ 4, 4, 8, 8, 8, 8, 4, 4},
		{20, 4,12,12,12,12, 4,20}
	};

	for(Position pos : possiblePosition){
		value += mobility[pos.x][pos.y]; // TODO do at mobility table
	}

	return value;
}

void printPosValue(const Position &pos,const int &value){
	printf("x = %d\ty = %d\tvalue=%d\n", pos.x, pos.y, value);
}

int alphaBeta(int depth, int color, const BitBoard &prevBoard, int alpha, int beta){
	SEARCH++;
	int bestValue = NEG_INF;
	if (depth <= 0){
		return getValue(color, prevBoard);
	}

#ifdef HASH
	int boardHash = prevBoard.getHash();
	if(depth <= hashTable[boardHash].depth){
		HASH_HIT++;
		if(prevBoard.isSame(hashTable[boardHash].board)){
			return hashTable[boardHash].value;
		}
		COLLISION++;
	}
#endif

	if(!canIGo(color, prevBoard)){
		if(!canIGo(color^1, prevBoard)){
			int winner = prevBoard.findWinner();
			if(winner == color)
				return GOOD;
			else if(winner == (color^1))
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
#ifdef HASH
	//	printPosValue(bestPos, bestValue);
//	if(hashTable[boardHash].depth < depth){
		hashTable[boardHash].depth = depth;
		hashTable[boardHash].value = bestValue;
		hashTable[boardHash].board.copy(prevBoard);
//	}
#endif

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
	printf("best move is %d %d, value = %d\n", bestMove.x, bestMove.y, bestValue);
	return bestMove;
}

inline void ai_go(int color, int depth){
	static unsigned int total_time = 0;
	unsigned int start_time = get_time();
	BOARD.go(minMax(depth, color), color);
	unsigned int end_time = get_time();
	unsigned int one_step_time = end_time - start_time;
	total_time += one_step_time;
	printf("this time used %u ms, total used %u ms\n", one_step_time, total_time);
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

// return time of unit msec
unsigned int get_time(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

