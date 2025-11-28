#include "PreComputed.hpp"
#include <iostream>
#include <bitset>

using namespace std;

// -------------------- TABLES --------------------
array<Bitboard, BOARD_SIZE> pawnAttacksWhite;
array<Bitboard, BOARD_SIZE> pawnAttacksBlack;
array<Bitboard, BOARD_SIZE> knightMoves;
array<Bitboard, BOARD_SIZE> kingMoves;

// -------------------- UTILS --------------------
string squareToString(Square sq) {
    int file = sq % 8;
    int rank = sq / 8;
    string s;
    s += char('a' + file);
    s += char('1' + rank);
    return s;
}

string squareToStringINT(int sq) {
    int file = sq % 8;    // 0 = A, ..., 7 = H
    int rank = sq / 8;    // 0 = rank 1, ..., 7 = rank 8

    string s;
    s += char('A' + file);
    s += char('1' + rank);
    return s;
}


void printBitboard(Bitboard bb) {
    for (int rank = 7; rank >= 0; --rank) {
        cout << rank + 1 << " ";
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            cout << ((bb >> sq) & 1ULL) << " ";
        }
        cout << "\n";
    }
    cout << "  A B C D E F G H\n";
}

// -------------------- PAWN ATTACKS --------------------
pair<int,int> PawnAttackCreation(Square pos1, Color color) {
    int pos = static_cast<int>(pos1);
    int row = pos / 8;
    int col = pos % 8;
    int left=-1, right=-1;

    if(color == WHITE){
        if(row < 7){
            if(col>0) left = pos + 7;
            if(col<7) right = pos + 9;
        }
    } else {
        if(row>0){
            if(col>0) left = pos - 9;
            if(col<7) right = pos - 7;
        }
    }
    return {left,right};
}

void InitPawnAttackTables(){
    for(int i=0;i<BOARD_SIZE;i++){
        pawnAttacksWhite[i] = 0;
        pawnAttacksBlack[i] = 0;

        auto [lW,rW] = PawnAttackCreation(static_cast<Square>(i), WHITE);
        if(lW!=-1) pawnAttacksWhite[i] |= 1ULL<<lW;
        if(rW!=-1) pawnAttacksWhite[i] |= 1ULL<<rW;

        auto [lB,rB] = PawnAttackCreation(static_cast<Square>(i), BLACK);
        if(lB!=-1) pawnAttacksBlack[i] |= 1ULL<<lB;
        if(rB!=-1) pawnAttacksBlack[i] |= 1ULL<<rB;
    }
}

// -------------------- KNIGHT --------------------
void InitKnightMoveTables(){
    int knightDeltas[8][2] = {{2,1},{1,2},{-1,2},{-2,1},{-2,-1},{-1,-2},{1,-2},{2,-1}};
    for(int i=0;i<BOARD_SIZE;i++){
        knightMoves[i] = 0;
        int row = i/8, col = i%8;
        for(auto &delta: knightDeltas){
            int r = row + delta[0];
            int c = col + delta[1];
            if(r>=0 && r<8 && c>=0 && c<8){
                knightMoves[i] |= 1ULL<<(r*8+c);
            }
        }
    }
}

// -------------------- KING --------------------
void InitKingMoveTables(){
    int kingDeltas[8][2] = {{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
    for(int i=0;i<BOARD_SIZE;i++){
        kingMoves[i]=0;
        int row=i/8, col=i%8;
        for(auto &delta: kingDeltas){
            int r = row + delta[0];
            int c = col + delta[1];
            if(r>=0 && r<8 && c>=0 && c<8){
                kingMoves[i] |= 1ULL<<(r*8+c);
            }
        }
    }
}

// -------------------- INIT ALL --------------------
void InitAllAttackTables(){
    InitPawnAttackTables();
    InitKnightMoveTables();
    InitKingMoveTables();
}
