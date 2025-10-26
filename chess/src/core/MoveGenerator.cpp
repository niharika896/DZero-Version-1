#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Position.hpp"
#include "magicBitboard.hpp"

// Global/static instance of magicBitboard
static magicBitboard Magic;

void InitMagicOnce() {
    static bool initialized = false;
    if (!initialized) {
        Magic.initMagicTables();
        initialized = true;
    }
}

class MoveGen{
    public:
        Bitboard GenerateAttacks(Square sq,Piece pieceType,Color color,const Bitboard* occupancy){
            switch(pieceType){
                case WHITE_PAWN:
                case BLACK_PAWN:
                    return(color==WHITE)?pawnAttacksWhite[sq]:pawnAttacksBlack[sq];
                case WHITE_KNIGHT:
                case BLACK_KNIGHT:
                    return knightMoves[sq];
                case WHITE_KING:
                case BLACK_KING:
                    return kingMoves[sq];
                case WHITE_BISHOP:
                case BLACK_BISHOP:{
                    
                    return Magic.getBishopAttacks(sq,*occupancy);
                }            
                case WHITE_ROOK:
                case BLACK_ROOK:{
                    
                    return Magic.getRookAttacks(sq,*occupancy);
                }
                case WHITE_QUEEN:
                case BLACK_QUEEN:{
                    
                    return Magic.getBishopAttacks(sq,*occupancy)| Magic.getRookAttacks(sq,*occupancy);
                }
                default:
                    return 0ULL;
            }
        }

};
int main(){
    InitAllAttackTables();  // pawns, knights, kings
    InitMagicOnce();        // sliders
    MoveGen move;
    
    Bitboard occupancy = 0ULL;
    occupancy |= 1ULL << D5;
    occupancy |= 1ULL << F5;

    Square sq = E4;
    Bitboard attacks = move.GenerateAttacks(sq, WHITE_KNIGHT, WHITE, &occupancy);

    printBitboard(attacks);
}