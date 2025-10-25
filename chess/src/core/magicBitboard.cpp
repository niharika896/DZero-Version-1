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
    0x1002004102008200, 0x1002004102008200, 0x4310002248214800, 0x402010c110014208, 0xa000a06240114001, 0xa000a06240114001, 0x402010c110014208, 0xa000a06240114001,
    0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x100c009840001000, 0x4310002248214800, 0xa000a06240114001, 0x4310002248214800,
    0x4310002248214800, 0x822143005020a148, 0x0001901c00420040, 0x0880504024308060, 0x0100201004200002, 0xa000a06240114001, 0x822143005020a148, 0x1002004102008200,
    0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x2008080100820102, 0x1481010004104010, 0x0002052000100024, 0xc880221002060081, 0xc880221002060081,
    0x4310002248214800, 0xc880221002060081, 0x0001901c00420040, 0x8400208020080201, 0x000e008400060020, 0x00449210e3902028, 0x402010c110014208, 0xc880221002060081,
    0x100c009840001000, 0xc880221002060081, 0x1000820800c00060, 0x2803101084008800, 0x2200608200100080, 0x0040900130840090, 0x0024010008800a00, 0x0400110410804810,
    0x402010c110014208, 0xa000a06240114001, 0xa000a06240114001, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200,
    0xa000a06240114001, 0x4310002248214800, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200, 0x1002004102008200
};

const Bitboard rookMagics[64] = {
    0x8200108041020020, 0x8200108041020020, 0xc880221002060081, 0x0009100804021000, 0x0500010004107800, 0x0024010008800a00, 0x0400110410804810, 0x8300038100004222,
    0x004a800182c00020, 0x0009100804021000, 0x3002200010c40021, 0x0020100104000208, 0x01021001a0080020, 0x0884020010082100, 0x1000820800c00060, 0x8020480110020020,
    0x0002052000100024, 0x0200190040088100, 0x0030802001a00800, 0x8010002004000202, 0x0040010100080010, 0x2200608200100080, 0x0001901c00420040, 0x0001400a24008010,
    0x1400a22008001042, 0x8200108041020020, 0x2004500023002400, 0x8105100028001048, 0x8010024d00014802, 0x8000820028030004, 0x402010c110014208, 0x8300038100004222,
    0x0001804002800124, 0x0084022014041400, 0x0030802001a00800, 0x0110a01001080008, 0x0b10080850081100, 0x000010040049020c, 0x0024010008800a00, 0x014c800040100426,
    0x1100400010208000, 0x0009100804021000, 0x0010024871202002, 0x8014001028c80801, 0x1201082010a00200, 0x0002008004102009, 0x8300038100004222, 0x0000401001a00408,
    0x4520920010210200, 0x0400110410804810, 0x8105100028001048, 0x8105100028001048, 0x0802801009083002, 0x8200108041020020, 0x8200108041020020, 0x4000a12400848110,
    0x2000804026001102, 0x2000804026001102, 0x800040a010040901, 0x80001802002c0422, 0x0010b018200c0122, 0x200204802a080401, 0x8880604201100844, 0x80000cc281092402
};
    Bitboard generateBishopMask(int sq)
    {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard mask = 0ULL;
        for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
        {
            mask |= 1ULL << r * 8 + f;
        }
        for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
        {
            mask |= 1ULL << r * 8 + f;
        }
        for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
        {
            mask |= 1ULL << r * 8 + f;
        }
        for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
        {
            mask |= 1ULL << r * 8 + f;
        }
        return mask;
    }
    Bitboard generateRookMask(int sq)
    {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard mask = 0ULL;
        for (int r = rank + 1; r <= 6; r++)
            mask |= 1ULL << r * 8 + file;
        for (int r = rank - 1; r >= 1; r--)
            mask |= 1ULL << r * 8 + file;
        for (int f = file + 1; f <= 6; f++)
            mask |= 1ULL << rank * 8 + f;
        for (int f = file - 1; f >= 1; f--)
            mask |= 1ULL << rank * 8 + f;
        return mask;
    }
    vector<Bitboard> generateOccupancyVariations(Bitboard mask)
    {
        vector<int> relevantSquares;
        for (int sq = 0; sq < 64; sq++)
        {
            if (mask & (1ULL << sq))
            {
                relevantSquares.push_back(sq);
            }
        }
        int numBits = relevantSquares.size();
        int numVariation = 1 << numBits;
        vector<Bitboard> occupancies(numVariation, 0ULL);

        for (int i = 0; i < numVariation; i++)
        {
            Bitboard occ = 0ULL;
            for (int j = 0; j < numBits; j++)
            {
                if (i & (1 << j))
                    occ |= 1ULL << relevantSquares[j];
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
            int s = r * 8 + f;
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break; // stop at blocker
        }

        // NW direction
        for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
        {
            int s = r * 8 + f;
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // SE direction
        for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
        {
            int s = r * 8 + f;
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // SW direction
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
        {
            int s = r * 8 + f;
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
            int s = r * 8 + file;
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // Down (rank decreasing)
        for (int r = rank - 1; r >= 0; r--)
        {
            int s = r * 8 + file;
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // Right (file increasing)
        for (int f = file + 1; f <= 7; f++)
        {
            int s = rank * 8 + f;
            attacks |= 1ULL << s;
            if (blockers & (1ULL << s))
                break;
        }

        // Left (file decreasing)
        for (int f = file - 1; f >= 0; f--)
        {
            int s = rank * 8 + f;
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
            int index = (occ * rookMagics[sq]) >> (64 - rookTable[sq].relevantBits);
            rookTable[sq].attacks[index] = generateRookAttack(sq, occ);
        }

        // ----- Bishop -----
        bishopTable[sq].mask = generateBishopMask(sq);
        occs = generateOccupancyVariations(bishopTable[sq].mask);
        bishopTable[sq].relevantBits = __builtin_popcountll(bishopTable[sq].mask);
        bishopTable[sq].magic = bishopMagics[sq];
        bishopTable[sq].attacks.resize(1 << bishopTable[sq].relevantBits);

        for (auto occ : occs) {
            int index = (occ * bishopMagics[sq]) >> (64 - bishopTable[sq].relevantBits);
            bishopTable[sq].attacks[index] = generateBishopAttack(sq, occ);
        }
    }
}
inline Bitboard getBishopAttacks(int sq, Bitboard occ) {
    occ &= bishopTable[sq].mask;
    int index = (occ * bishopTable[sq].magic) >> (64 - bishopTable[sq].relevantBits);
    return bishopTable[sq].attacks[index];
}

inline Bitboard getRookAttacks(int sq, Bitboard occ) {
    occ &= rookTable[sq].mask;
    int index = (occ * rookTable[sq].magic) >> (64 - rookTable[sq].relevantBits);
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
void printBitboard(Bitboard bb) {
    cout << "\n    a b c d e f g h\n";
    cout << "   -----------------\n";
    for (int rank = 7; rank >= 0; --rank) {
        cout << rank + 1 << " | ";
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            cout << ((bb >> sq) & 1ULL ? "● " : ". ");
        }
        cout << "|" << endl;
    }
    cout << "   -----------------\n\n";
}
int main()
{
    magicBitboard magic;
    Bitboard mask = magic.generateBishopMask(E4);

    magic.printBitboard(mask);
    vector<Bitboard> occ = magic.generateOccupancyVariations(mask);
    magic.printBitboard(occ[20]);
    cout<<"---------------"<<endl;
    magic.initMagicTables();
    Bitboard blockers = 0ULL;
    blockers |= 1ULL << D5; // example blocker
    blockers |= 1ULL << F5;

    cout << "Bishop attacks from E4 with blockers:\n";
    printBitboard(magic.getBishopAttacks(E4, blockers));

    cout << "Rook attacks from D4 with blockers:\n";
    printBitboard(magic.getRookAttacks(D4, blockers));
}