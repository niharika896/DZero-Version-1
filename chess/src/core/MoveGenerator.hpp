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
class MoveGen {
public:
    Bitboard GenerateAttacks(
        Square sq,
        Piece pieceType,
        Color color,
        const Bitboard* occupancy,
        const Bitboard* whitePieces,
        const Bitboard* blackPieces
    ) {
        Bitboard attacks = 0ULL;

        switch (pieceType) {
            case WHITE_PAWN:
            case BLACK_PAWN:
                attacks = (color == WHITE) ? pawnAttacksWhite[sq] : pawnAttacksBlack[sq];
                break;

            case WHITE_KNIGHT:
            case BLACK_KNIGHT:
                attacks = knightMoves[sq];
                break;

            case WHITE_KING:
            case BLACK_KING:
                attacks = kingMoves[sq];
                break;

            case WHITE_BISHOP:
            case BLACK_BISHOP:
                attacks = Magic.getBishopAttacks(sq, *occupancy);
                break;

            case WHITE_ROOK:
            case BLACK_ROOK:
                attacks = Magic.getRookAttacks(sq, *occupancy);
                break;

            case WHITE_QUEEN:
            case BLACK_QUEEN:
                attacks = Magic.getBishopAttacks(sq, *occupancy) |
                          Magic.getRookAttacks(sq, *occupancy);
                break;

            default:
                return 0ULL;
        }

        // ---------------------------------------------------------
        // REMOVE SAME COLOR PIECES!
        // ---------------------------------------------------------
        Bitboard friendly = (color == WHITE) ? *whitePieces : *blackPieces;
        attacks &= ~friendly;

        return attacks;
    }
};
