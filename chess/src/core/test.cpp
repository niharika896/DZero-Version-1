#include <iostream>
#include <vector>
#include "BitBoard.hpp"     // your bitboard print helper
#include "magicBitboard.hpp"

using namespace std;

void printBitboard(Bitboard bb) {
    for (int r = 7; r >= 0; r--) {
        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            cout << ((bb >> sq) & 1);
        }
        cout << "\n";
    }
    cout << "\n";
}
/*
int main() {
    magicBitboard mb;

    int testSquare = 36;   // e4 = square 36 (change this if needed)
    cout << "Testing bishop at square: " << testSquare << "\n\n";

    // 1. Generate bishop mask
    Bitboard mask = mb.generateBishopMask(testSquare);
    cout << "Bishop Mask:\n";
    printBitboard(mask);

    // 2. Generate all occupancies inside mask
    vector<Bitboard> occs = mb.generateOccupancyVariations(mask);

    cout << "Total occupancy variations = " << occs.size() << "\n\n";

    // 3. For each occupancy variation: print occupancy + bishop attacks
    int index = 0;
    for (Bitboard occ : occs) {
        cout << "------------------------------\n";
        cout << "Occupancy #" << index++ << ":\n";
        printBitboard(occ);

        Bitboard attacks = mb.generateBishopAttack(testSquare, occ);

        cout << "Bishop Attacks:\n";
        printBitboard(attacks);
    }

    return 0;
}*/
