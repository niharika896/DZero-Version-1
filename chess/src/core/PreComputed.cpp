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
string squareToString(Square sq) {
    int file = sq % 8;
    int rank = sq / 8;
    string s;
    s += char('a' + file);
    s += char('1' + rank);
    return s;
}

void PawnAttack(Square pos1, Color color) {
    int pos = static_cast<int>(pos1);
    int row = pos / 8;
    int col = pos % 8;

    int leftAttack = -1, rightAttack = -1;

    if (color == WHITE) {
        if (row < 7) { // white moves up the board
            if (col > 0) leftAttack = pos + 7;   // top-left
            if (col < 7) rightAttack = pos + 9;  // top-right
        }
    } else {
        if (row > 0) { // black moves down
            if (col > 0) leftAttack = pos - 9;   // bottom-left
            if (col < 7) rightAttack = pos - 7;  // bottom-right
        }
    }
cout << "Pawn at " << squareToString(pos1) << " attacks: ";
    if (leftAttack != -1) cout << squareToString((Square)leftAttack) << " ";
    if (rightAttack != -1) cout << squareToString((Square)rightAttack);
    cout << endl;
}

int main(){
    PawnAttack(E4,WHITE);
    PawnAttack(H4,BLACK);
}
