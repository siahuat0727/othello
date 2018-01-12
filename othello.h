#ifndef OTHELLO_H
#define OTHELLO_H

#include <vector>
#include "bitboard.h"

using namespace std;

BitBoard BOARD;

unsigned int get_time();
void whereCanGo(int color, BitBoard board=BOARD);
void printVector(vector<Position> V);
bool canIGo(int color, const BitBoard &board=BOARD);
Position minMax(int depth, int color);
int alphaBeta(int depth, int color, const BitBoard &prevBoard, int alpha, int beta);
int getValue(int color);
void ai_go(int color, int depth=8);
unsigned int get_time();
void hm_vs_ai(const int ai_color, BitBoard &board=BOARD, int depth=9);
void ai_vs_ai(int depth);

#endif
