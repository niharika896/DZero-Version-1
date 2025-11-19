#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Position.hpp"
#include "magicBitboard.hpp"

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

    // ------------------------------------------------------
    // Generate ALL moves for a pawn: pushes + captures
    // ------------------------------------------------------
    Bitboard GeneratePawnMoves(
        Square sq,
        Color color,
        const Bitboard* occupancy,
        const Bitboard* whitePieces,
        const Bitboard* blackPieces,
        int enPassantSq
    ) {
        Bitboard moves = 0ULL;
        Bitboard opp = (color == WHITE) ? *blackPieces : *whitePieces;

        int direction = (color == WHITE) ? 8 : -8;
        int rank = sq / 8;
        int file = sq % 8;

        // ------------------------------
        // 1-step forward
        // ------------------------------
        int one = sq + direction;
        if (one >= 0 && one < 64 && !(*occupancy & (1ULL << one))) {
            moves |= (1ULL << one);

            // ------------------------------
            // 2-step forward (only from rank 2 or 7)
            // ------------------------------
            bool canStart = (color == WHITE && rank == 1) || (color == BLACK && rank == 6);
            if (canStart) {
                int two = sq + 2 * direction;
                if (!( *occupancy & (1ULL << two) ))
                    moves |= (1ULL << two);
            }
        }

        // ------------------------------
        // CAPTURES
        // ------------------------------
        int diagL = sq + direction - 1;
        int diagR = sq + direction + 1;

        // ● LEFT capture
        if (file > 0 && diagL >= 0 && diagL < 64 && (opp & (1ULL << diagL)))
            moves |= (1ULL << diagL);

        // ● RIGHT capture
        if (file < 7 && diagR >= 0 && diagR < 64 && (opp & (1ULL << diagR)))
            moves |= (1ULL << diagR);

        // ------------------------------
        // En Passant
        // ------------------------------
        if (enPassantSq != -1) {
            if (file > 0 && diagL == enPassantSq) moves |= (1ULL << diagL);
            if (file < 7 && diagR == enPassantSq) moves |= (1ULL << diagR);
        }

        return moves;
    }

    // ------------------------------------------------------
    // Unified generator
    // ------------------------------------------------------
    Bitboard GenerateAttacks(
        Square sq,
        Piece pieceType,
        Color color,
        const Bitboard* occupancy,
        const Bitboard* whitePieces,
        const Bitboard* blackPieces,
        int enPassantSq = -1   // <-- ADDED for pawn en-passant support
    ) {
        Bitboard attacks = 0ULL;

        switch (pieceType) {

            // ------------------------------------------------------
            // PAWNS → use our custom generator (pushes + captures)
            // ------------------------------------------------------
            case WHITE_PAWN:
            case BLACK_PAWN:
                return GeneratePawnMoves(
                    sq,
                    color,
                    occupancy,
                    whitePieces,
                    blackPieces,
                    enPassantSq
                );

            // ------------------------------------------------------
            // KNIGHT
            // ------------------------------------------------------
            case WHITE_KNIGHT:
            case BLACK_KNIGHT:
                attacks = knightMoves[sq];
                break;

            // ------------------------------------------------------
            // KING
            // ------------------------------------------------------
            case WHITE_KING:
            case BLACK_KING:
                attacks = kingMoves[sq];
                break;

            // ------------------------------------------------------
            // SLIDING PIECES
            // ------------------------------------------------------
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

        // REMOVE friendly pieces
        Bitboard friendly = (color == WHITE) ? *whitePieces : *blackPieces;
        attacks &= ~friendly;

        return attacks;
    }
};
