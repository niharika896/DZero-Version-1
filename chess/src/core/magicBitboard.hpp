#pragma once   // or use traditional include guards

#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
#include "BitBoard.hpp"
#include "Position.hpp"
using namespace std;

class magicBitboard : public Bitboardhelper, public BitboardInitializer
{
public:
struct Magic {
    Bitboard mask;
    Bitboard magic;
    int relevantBits;
    vector<Bitboard> attacks;
};
    const Bitboard bishopMagics[64] = {
    0x1002004102008200, 0x1002004102008200, 0x4310002248214800, 0x402010c110014208, 0xfc0a66c64a7ef576, 0xa000a06240114001, 0x402010c110014208, 0xa000a06240114001,
    0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x100c009840001000, 0x4310002248214800, 0xa000a06240114001, 0x4310002248214800,
    0x4310002248214800, 0x822143005020a148, 0x0001901c00420040, 0x0880504024308060, 0x0100201004200002, 0xa000a06240114001, 0x822143005020a148, 0x1002004102008200,
    0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x2008080100820102, 0x1481010004104010, 0x0002052000100024, 0xc880221002060081, 0xc880221002060081,
    0x4310002248214800, 0xc880221002060081, 0x0001901c00420040, 0x8400208020080201, 0x000e008400060020, 0x00449210e3902028, 0x402010c110014208, 0xc880221002060081,
    0x100c009840001000, 0xc880221002060081, 0x1000820800c00060, 0x2803101084008800, 0x2200608200100080, 0x0040900130840090, 0x0024010008800a00, 0x0400110410804810,
    0x402010c110014208, 0xa000a06240114001, 0xa000a06240114001, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200,
    0xa000a06240114001, 0x4310002248214800, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200
};

Bitboard rookMagics[64] = 
{
0x0080001020400080, 0x0040001000200040, 0x0080081000200080, 0x0080040800100080,
0x0080020400080080, 0x0080010200040080, 0x0080008001000200, 0x0080002040800100,
0x0000800020400080, 0x0000400020005000, 0x0000801000200080, 0x0000800800100080,
0x0000800400080080, 0x0000800200040080, 0x0000800100020080, 0x0000800040800100,

0x0000208000400080, 0x0000404000201000, 0x0000808010002000, 0x0000808008001000,
0x0000808004000800, 0x0000808002000400, 0x0000010100020004, 0x0000020000408104,
0x0000208080004000, 0x0000200040005000, 0x0000100080200080, 0x0000080080100080,
0x0000040080080080, 0x0000020080040080, 0x0000010080800200, 0x0000800080004100,
0x0000204000800080, 0x0000200040401000, 0x0000100080802000, 0x0000080080801000,
0x0000040080800800, 0x0000020080800400, 0x0000020001010004, 0x0000800040800100,
0x0000204000808000, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
0x0000040008008080, 0x0000020004008080, 0x0000010002008080, 0x0000004081020004,
0x0000204000800080, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
0x0000040008008080, 0x0000020004008080, 0x0000800100020080, 0x0000800041000080,
0x00FFFCDDFCED714A, 0x007FFCDDFCED714A, 0x003FFFCDFFD88096, 0x0000040810002101,
0x0001000204080011, 0x0001000204000801, 0x0001000082000401, 0x0001FFFAABFAD1A2
};

     Bitboard generateBishopMask(int sq)
{
    int rank = sq / 8;
    int file = sq % 8;
    Bitboard mask = 0ULL;

    // NE
    for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
        mask |= 1ULL << (r * 8 + f);

    // NW
    for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
        mask |= 1ULL << (r * 8 + f);

    // SE
    for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
        mask |= 1ULL << (r * 8 + f);

    // SW
    for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
        mask |= 1ULL << (r * 8 + f);

    return mask;
}

    Bitboard generateRookMask(int sq)
    {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard mask = 0ULL;
        for (int r = rank + 1; r <= 6; r++)
            mask |= 1ULL << (r * 8 + file);
        for (int r = rank - 1; r >= 1; r--)
            mask |= 1ULL << (r * 8 + file);
        for (int f = file + 1; f <= 6; f++)
            mask |= 1ULL << (rank * 8 + f);
        for (int f = file - 1; f >= 1; f--)
            mask |= 1ULL << (rank * 8 + f);
        return mask;
    }
//If the rook mask for a square has n squares inside it, then there are: 2^n possible ways to place blockers on those n squares.Each configuration is called an occupancy variation. Magic bitboards require all of them to build the attack lookup table.

vector<Bitboard> generateOccupancyVariations(Bitboard mask)
    {
        vector<int> relevantSquares;  //array of all squares that are common with the generated mask
        for (int sq = 0; sq < 64; sq++)
        {
            if (mask & (1ULL << sq))
            {
                relevantSquares.push_back(sq);
            }
        }
        int numBits = relevantSquares.size(); //if numBits=3 say then num variations=8 = binary 000->111 => 3 bits are needed
        int numVariation = 1 << numBits; // 1 << numBits = 2^n which is the total number of possible blocker configurations
        vector<Bitboard> occupancies(numVariation, 0ULL); //array of bitboards of that length

        for (int i = 0; i < numVariation; i++) //for each variation
        {
            Bitboard occ = 0ULL; 
            for (int j = 0; j < numBits; j++)  //for each bit in that variation
            {
                if (i & (1 << j)) 
                    occ |= 1ULL << relevantSquares[j]; //If bit j of i is 1 → place a blocker at relevantSquares[j]
            }
            occupancies[i] = occ;
        }
        return occupancies;
    }
    
Bitboard generateBishopAttack(int sq, Bitboard blockers)
    {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard attacks = 0ULL;

        // NE direction
        for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++)
        {
            int s = (r * 8 + f);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break; // stop at blocker
        }

        // NW direction
        for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
        {
            int s = (r * 8 + f);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // SE direction
        for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
        {
            int s = (r * 8 + f);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // SW direction
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
        {
            int s = (r * 8 + f);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        return attacks;
    }
    
Bitboard generateRookAttack(int sq, Bitboard blockers)
    {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard attacks = 0ULL;

        // Up (rank increasing)
        for (int r = rank + 1; r <= 7; r++)
        {
            int s = (r * 8 + file);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // Down (rank decreasing)
        for (int r = rank - 1; r >= 0; r--)
        {
            int s = (r * 8 + file);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // Right (file increasing)
        for (int f = file + 1; f <= 7; f++)
        {
            int s = (rank * 8 + f);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // Left (file decreasing)
        for (int f = file - 1; f >= 0; f--)
        {
            int s = (rank * 8 + f);
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        return attacks;
    }
    
array<Magic, 64> bishopTable;
array<Magic, 64> rookTable;

void initMagicTables() {
    for (int sq = 0; sq < 64; sq++) {
        // ----- Rook -----
        rookTable[sq].mask = generateRookMask(sq);
        vector<Bitboard> occs = generateOccupancyVariations(rookTable[sq].mask);
        rookTable[sq].relevantBits = __builtin_popcountll(rookTable[sq].mask);
        rookTable[sq].magic = rookMagics[sq];
        rookTable[sq].attacks.resize(1 << rookTable[sq].relevantBits);

        for (auto occ : occs) {
            int index = (int)(((uint64_t)occ * rookTable[sq].magic) >> (64 - rookTable[sq].relevantBits));
            rookTable[sq].attacks[index] = generateRookAttack(sq, occ);
        }

        // ----- Bishop -----
        bishopTable[sq].mask = generateBishopMask(sq);
        occs = generateOccupancyVariations(bishopTable[sq].mask);
        bishopTable[sq].relevantBits = __builtin_popcountll(bishopTable[sq].mask);
        bishopTable[sq].magic = bishopMagics[sq];
        bishopTable[sq].attacks.resize(1 << bishopTable[sq].relevantBits);

        for (auto occ : occs) {
            occ &= bishopTable[sq].mask;
int index = (uint64_t)(occ * bishopTable[sq].magic) >> (64 - bishopTable[sq].relevantBits);

        }
    }
}

inline Bitboard getBishopAttacks(int sq, Bitboard occ) {
    occ &= bishopTable[sq].mask;
    int index = (occ * bishopTable[sq].magic) >> (64 - bishopTable[sq].relevantBits);
    bishopTable[sq].attacks[index] = generateBishopAttack(sq, occ); 
    return bishopTable[sq].attacks[index];
}

inline Bitboard getRookAttacks(int sq, Bitboard occ) {
    occ &= rookTable[sq].mask;
    int index = (int)(((uint64_t)occ * rookTable[sq].magic) >> (64 - rookTable[sq].relevantBits));

    return rookTable[sq].attacks[index];
}
void printBitboard(Bitboard bb) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            cout << ((bb >> sq) & 1ULL);
        }
        cout << "\n";
    }
    cout << "\n";
}

};
