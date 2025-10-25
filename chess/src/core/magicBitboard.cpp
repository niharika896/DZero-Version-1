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
};
int main()
{
    magicBitboard magic;
    Bitboard mask = magic.generateBishopMask(E4);

    magic.printBitboard(mask);
    vector<Bitboard> occ = magic.generateOccupancyVariations(mask);
    magic.printBitboard(occ[20]);
    cout<<"---------------"<<endl;
    int sq = E4; // example square
Bitboard bishopMask = magic.generateBishopMask(sq);
vector<Bitboard> occupancies = magic.generateOccupancyVariations(bishopMask);

vector<Bitboard> bishopAttacks(occupancies.size());
for (size_t i = 0; i < occupancies.size(); i++) {
    bishopAttacks[i] = magic.generateBishopAttack(sq, occupancies[i]);
}
    magic.printBitboard(occupancies[50]);
    magic.printBitboard(bishopAttacks[50]);

}