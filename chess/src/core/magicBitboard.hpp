#pragma once

#include <iostream>
#include <cstdint>
#include <array>
#include <vector>
#include <bitset>
#include "BitBoard.hpp"
#include "Position.hpp"

using std::array;
using std::vector;
using std::uint64_t;
using std::cout;
using std::endl;
//correct code below
class magicBitboard : public Bitboardhelper, public BitboardInitializer {
public:
    struct Magic {
        Bitboard mask;
        Bitboard magic;
        int relevantBits;
        vector<Bitboard> attacks;
    };

    // Tested magic numbers from common magic-bitboard tables
    // (rook and bishop 64-entry tables)
    static constexpr Bitboard bishopMagics[64] = {
        0x40040844404084ULL,0x2004208a004208ULL,0x10190041080202ULL,0x108060845042010ULL,
        0x581104180800210ULL,0x2112080446200010ULL,0x1080820820060210ULL,0x3c0808410220200ULL,
        0x4050404440404ULL,0x21001420088ULL,0x24d0080801082102ULL,0x1020a0a020400ULL,
        0x40308200402ULL,0x4011002100800ULL,0x401484104104005ULL,0x801010402020200ULL,
        0x400210c3880100ULL,0x404022024108200ULL,0x810018200204102ULL,0x4002801a02003ULL,
        0x85040820080400ULL,0x810102c808880400ULL,0xe900410884800ULL,0x8002020480840102ULL,
        0x220200865090201ULL,0x2010100a02021202ULL,0x152048408022401ULL,0x20080002081110ULL,
        0x4001001021004000ULL,0x800040400a011002ULL,0xe4004081011002ULL,0x1c004001012080ULL,
        0x8004200962a00220ULL,0x8422100208500202ULL,0x2000402200300c08ULL,0x8646020080080080ULL,
        0x80020a0200100808ULL,0x2010004880111000ULL,0x623000a080011400ULL,0x42008c0340209202ULL,
        0x209188240001000ULL,0x400408a884001800ULL,0x110400a6080400ULL,0x1840060a44020800ULL,
        0x90080104000041ULL,0x201011000808101ULL,0x1a2208080504f080ULL,0x8012020600211212ULL,
        0x500861011240000ULL,0x180806108200800ULL,0x4000020e01040044ULL,0x300000261044000aULL,
        0x802241102020002ULL,0x20906061210001ULL,0x5a84841004010310ULL,0x4010801011c04ULL,
        0xa010109502200ULL,0x4a02012000ULL,0x500201010098b028ULL,0x8040002811040900ULL,
        0x28000010020204ULL,0x6000020202d0240ULL,0x8918844842082200ULL,0x4010011029020020ULL
    };

    static constexpr Bitboard rookMagics[64] = {
        0x8a80104000800020ULL,0x140002000100040ULL,0x2801880a0017001ULL,0x100081001000420ULL,
        0x200020010080420ULL,0x3001c0002010008ULL,0x8480008002000100ULL,0x2080088004402900ULL,
        0x800098204000ULL,0x2024401000200040ULL,0x100802000801000ULL,0x120800800801000ULL,
        0x208808088000400ULL,0x2802200800400ULL,0x2200800100020080ULL,0x801000060821100ULL,
        0x80044006422000ULL,0x100808020004000ULL,0x12108a0010204200ULL,0x140848010000802ULL,
        0x481828014002800ULL,0x8094004002004100ULL,0x4010040010010802ULL,0x20008806104ULL,
        0x100400080208000ULL,0x2040002120081000ULL,0x21200680100081ULL,0x20100080080080ULL,
        0x2000a00200410ULL,0x20080800400ULL,0x80088400100102ULL,0x80004600042881ULL,
        0x4040008040800020ULL,0x440003000200801ULL,0x4200011004500ULL,0x188020010100100ULL,
        0x14800401802800ULL,0x2080040080800200ULL,0x124080204001001ULL,0x200046502000484ULL,
        0x480400080088020ULL,0x1000422010034000ULL,0x30200100110040ULL,0x100021010009ULL,
        0x2002080100110004ULL,0x202008004008002ULL,0x20020004010100ULL,0x2048440040820001ULL,
        0x101002200408200ULL,0x40802000401080ULL,0x4008142004410100ULL,0x2060820c0120200ULL,
        0x1001004080100ULL,0x20c020080040080ULL,0x2935610830022400ULL,0x44440041009200ULL,
        0x280001040802101ULL,0x2100190040002085ULL,0x80c0084100102001ULL,0x4024081001000421ULL
    };

    array<Magic,64> bishopTable;
    array<Magic,64> rookTable;

    // ---------- mask generators ----------
    Bitboard generateBishopMask(int sq) {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard mask = 0ULL;

        // NE
        for (int r = rank + 1, f = file + 1; r < 7 && f < 7; ++r, ++f) mask |= (1ULL << (r*8 + f));
        // NW
        for (int r = rank + 1, f = file - 1; r < 7 && f > 0; ++r, --f) mask |= (1ULL << (r*8 + f));
        // SE
        for (int r = rank - 1, f = file + 1; r > 0 && f < 7; --r, ++f) mask |= (1ULL << (r*8 + f));
        // SW
        for (int r = rank - 1, f = file - 1; r > 0 && f > 0; --r, --f) mask |= (1ULL << (r*8 + f));

        return mask;
    }

    Bitboard generateRookMask(int sq) {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard mask = 0ULL;

        for (int r = rank + 1; r < 7; ++r) mask |= (1ULL << (r*8 + file));
        for (int r = rank - 1; r > 0; --r)       mask |= (1ULL << (r*8 + file));
        for (int f = file + 1; f < 7; ++f)       mask |= (1ULL << (rank*8 + f));
        for (int f = file - 1; f > 0; --f)       mask |= (1ULL << (rank*8 + f));

        return mask;
    }

    // generate occupancy variations (2^n)
    vector<Bitboard> generateOccupancyVariations(Bitboard mask) {
        vector<int> squares;
        for (int sq = 0; sq < 64; ++sq)
            if (mask & (1ULL << sq)) squares.push_back(sq);

        int n = (int)squares.size();
        int variations = 1 << n;
        vector<Bitboard> out;
        out.reserve(variations);

        for (int i = 0; i < variations; ++i) {
            Bitboard occ = 0ULL;
            for (int b = 0; b < n; ++b)
                if (i & (1 << b))
                    occ |= (1ULL << squares[b]);
            out.push_back(occ);
        }
        return out;
    }

    // generate sliding attacks with blockers (used to fill tables)
    Bitboard generateBishopAttack(int sq, Bitboard blockers) {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard attacks = 0ULL;

        // NE
        for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; ++r, ++f) {
            int s = r*8 + f;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        // NW
        for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; ++r, --f) {
            int s = r*8 + f;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        // SE
        for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; --r, ++f) {
            int s = r*8 + f;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        // SW
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
            int s = r*8 + f;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        return attacks;
    }

    Bitboard generateRookAttack(int sq, Bitboard blockers) {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard attacks = 0ULL;

        // Up
        for (int r = rank + 1; r <= 7; ++r) {
            int s = r*8 + file;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        // Down
        for (int r = rank - 1; r >= 0; --r) {
            int s = r*8 + file;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        // Right
        for (int f = file + 1; f <= 7; ++f) {
            int s = rank*8 + f;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        // Left
        for (int f = file - 1; f >= 0; --f) {
            int s = rank*8 + f;
            attacks |= (1ULL << s);
            if (blockers & (1ULL << s)) break;
        }
        return attacks;
    }

    // initialize tables
    void initMagicTables() {
        for (int sq = 0; sq < 64; ++sq) {
            // ROOK
            rookTable[sq].mask = generateRookMask(sq);
            rookTable[sq].relevantBits = __builtin_popcountll(rookTable[sq].mask);
            rookTable[sq].magic = rookMagics[sq];
            rookTable[sq].attacks.assign(1u << rookTable[sq].relevantBits, 0ULL);

            {
                auto occs = generateOccupancyVariations(rookTable[sq].mask);
                for (auto occ : occs) {
                    size_t index = (size_t)(((uint64_t)occ * rookTable[sq].magic) >> (64 - rookTable[sq].relevantBits));
                    rookTable[sq].attacks[index] = generateRookAttack(sq, occ);
                }
            }

            // BISHOP
            bishopTable[sq].mask = generateBishopMask(sq);
            bishopTable[sq].relevantBits = __builtin_popcountll(bishopTable[sq].mask);
            bishopTable[sq].magic = bishopMagics[sq];
            bishopTable[sq].attacks.assign(1u << bishopTable[sq].relevantBits, 0ULL);

            {
                auto occs = generateOccupancyVariations(bishopTable[sq].mask);
                for (auto occ : occs) {
                    occ &= bishopTable[sq].mask;
                    size_t index = (size_t)(((uint64_t)occ * bishopTable[sq].magic) >> (64 - bishopTable[sq].relevantBits));
                    bishopTable[sq].attacks[index] = generateBishopAttack(sq, occ);
                }
            }
        }
    }

    inline Bitboard getBishopAttacks(int sq, Bitboard occ) {
        occ &= bishopTable[sq].mask;
        size_t index = (size_t)(((uint64_t)occ * bishopTable[sq].magic) >> (64 - bishopTable[sq].relevantBits));
        return bishopTable[sq].attacks[index];
    }

    inline Bitboard getRookAttacks(int sq, Bitboard occ) {
        occ &= rookTable[sq].mask;
        size_t index = (size_t)(((uint64_t)occ * rookTable[sq].magic) >> (64 - rookTable[sq].relevantBits));
        return rookTable[sq].attacks[index];
    }

    void printBitboard(Bitboard bb) {
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 0; file < 8; ++file) {
                int sq = rank*8 + file;
                cout << (((bb >> sq) & 1ULL) ? '1' : '.');
            }
            cout << '\n';
        }
        cout << '\n';
    }
};
