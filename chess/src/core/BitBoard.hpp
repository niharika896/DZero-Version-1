#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
#include "../core/PreComputedTables/PreComputed.hpp"
using namespace std;
using Bitboard = uint64_t;



// Enumeration for chess pieces for clarity 13-is for null piece
enum Piece {
    // Individual white pieces
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    // Individual black pieces
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    // Aggregates
    WHITE_PIECES,   // all white pieces
    BLACK_PIECES,   // all black pieces
    ALL_PIECES,     // all pieces on board
    NO_PIECE        // optional placeholder
};



class BitboardInitializer{
    public:
        array<Bitboard, 15> pieces{}; // Bitboards for each piece type
        array<Bitboard, 64> squareMasks{}; // Masks for each square

        BitboardInitializer() {
            initSquareMasks();
            initPieces();
        }

        void initSquareMasks(){
            for(int i=0;i<64;i++){
                squareMasks[i]=1ULL<<i;
            }
        }
        void initPieces() {
        // White pieces
        pieces[WHITE_PAWN]   = 0x000000000000FF00ULL;
        pieces[WHITE_ROOK]   = 0x0000000000000081ULL;
        pieces[WHITE_KNIGHT] = 0x0000000000000042ULL;
        pieces[WHITE_BISHOP] = 0x0000000000000024ULL;
        pieces[WHITE_QUEEN]  = 0x0000000000000008ULL;
        pieces[WHITE_KING]   = 0x0000000000000010ULL;

        // Black pieces
        pieces[BLACK_PAWN]   = 0x00FF000000000000ULL;
        pieces[BLACK_ROOK]   = 0x8100000000000000ULL;
        pieces[BLACK_KNIGHT] = 0x4200000000000000ULL;
        pieces[BLACK_BISHOP] = 0x2400000000000000ULL;
        pieces[BLACK_QUEEN]  = 0x0800000000000000ULL;
        pieces[BLACK_KING]   = 0x1000000000000000ULL;
        pieces[WHITE_PIECES] = 0;
        pieces[BLACK_PIECES] = 0;

        for (int i = WHITE_PAWN; i <= WHITE_KING; ++i)
            pieces[WHITE_PIECES] |= pieces[i];

        for (int i = BLACK_PAWN; i <= BLACK_KING; ++i)
            pieces[BLACK_PIECES] |= pieces[i];

        // All pieces
        pieces[ALL_PIECES] = pieces[WHITE_PIECES] | pieces[BLACK_PIECES];
    }
        void printBitboard(Bitboard bb) {
    bitset<64> b(bb);
    for (int rank = 7; rank >= 0; --rank) {    // rank 8 to rank 1
        for (int file = 0; file < 8; ++file) { // file A to H
            int sq = rank * 8 + file;          // calculate square index
            cout << b[sq];
        }
        cout << "\n";
    }
    cout << "\n";
}

        void movePiece(Piece p,Square src,Square dst){
            Bitboard srcMask=squareMasks[src];
            Bitboard dstMask=squareMasks[dst];
            //clearing piece from src
            pieces[p]&=~srcMask;

            //clearinf the dst peice from all bitboards to maintain consistency
            for (int i = WHITE_PAWN; i <= BLACK_KING; ++i) {
                pieces[i] &= ~dstMask;
            }

            //Placing piece at this dst
            pieces[p]|=dstMask;
            pieces[WHITE_PIECES] = 0;
            pieces[BLACK_PIECES] = 0;
            for (int i = WHITE_PAWN; i <= WHITE_KING; ++i) pieces[WHITE_PIECES] |= pieces[i];
            for (int i = BLACK_PAWN; i <= BLACK_KING; ++i) pieces[BLACK_PIECES] |= pieces[i];

            // Update total board occupancy
            pieces[ALL_PIECES] = pieces[WHITE_PIECES] | pieces[BLACK_PIECES];

        }
};
class Bitboardhelper {
    public:
        static Bitboard countSetBits(Bitboard bb) {
            return static_cast<int>(__builtin_popcountll(bb));
        }
        static Bitboard getLSBIndex(Bitboard bb) {
            if (bb == 0) return -1; // No bits are set
            return __builtin_ctzll(bb);
        }
        static Bitboard getMSBIndex(Bitboard bb) {
            if (bb == 0) return -1; // No bits are set
            return 63 - __builtin_clzll(bb);
        }
        static Bitboard popLSB(Bitboard &bb) {
            Bitboard lsb = bb & -bb; // Isolate LSB
            bb &= bb - 1;            // Clear LSB
            return lsb;
        }
        static Bitboard setBit(Bitboard &bb, int index) {
            bb |= (1ULL << index);
            return index;
        }
        static Bitboard clearBit(Bitboard &bb, int index) {
            bb &= ~(1ULL << index);
            return index;
        }
    };
    

