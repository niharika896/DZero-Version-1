#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Position.hpp"
using namespace std;


int main() {
    BitboardInitializer board;
    
    
    
    //InitAllAttackTables();
    cout << "Hello, chess magic bitboards!\n";
cout << "E4 = " << E4 << ", BLACK_PAWN = " << BLACK_PAWN << endl;

    cout<<"Pawn Attacks White from E4: ";
    /*printBitboard(pawnAttacksWhite[E4]);
    
    cout<<"ALL Pieces Initial Position:\n";
    for (const auto& pos : BitboardUtils::getAllPiecePositions(board.pieces[BLACK_PAWN])) {
        cout << pos << " ";
    }*/
}

