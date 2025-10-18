#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
using namespace std;
using Bitboard = uint64_t;

constexpr int BOARD_SIZE = 64;

enum Square {
    // Rank 1
    A1, B1, C1, D1, E1, F1, G1, H1,
    // Rank 2
    A2, B2, C2, D2, E2, F2, G2, H2,
    // Rank 3
    A3, B3, C3, D3, E3, F3, G3, H3,
    // Rank 4
    A4, B4, C4, D4, E4, F4, G4, H4,
    // Rank 5
    A5, B5, C5, D5, E5, F5, G5, H5,
    // Rank 6
    A6, B6, C6, D6, E6, F6, G6, H6,
    // Rank 7
    A7, B7, C7, D7, E7, F7, G7, H7,
    // Rank 8
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
};

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

enum Color {
    WHITE, BLACK
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
        for (int i = 63; i >= 0; --i) {
            cout << b[i];
            if (i % 8 == 0) cout << "\n";
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
int main() {
    BitboardInitializer board;
    
    cout << "White pawns:\n";
    board.printBitboard(board.pieces[WHITE_PAWN]);
    board.movePiece(WHITE_PAWN,E2,E4);
    cout << "White pawns:\n";
    board.printBitboard(board.pieces[WHITE_PAWN]);
    cout<<"white pieces:\n";
    board.printBitboard(board.pieces[WHITE_PIECES]);
    cout<<"All pieces:\n";
    board.printBitboard(board.pieces[ALL_PIECES]);
}

