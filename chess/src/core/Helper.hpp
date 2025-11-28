#pragma once
#include <iostream>
#include <cstdint>
#include <array>
#include <bitset>
#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Position.hpp"
#include "magicBitboard.hpp"

class Helper{
    private:
        Piece charToPiece(char c){
            switch(c){
                case 'P': return WHITE_PAWN;
                case 'N': return WHITE_KNIGHT;
                case 'B': return WHITE_BISHOP;
                case 'R': return WHITE_ROOK;
                case 'Q': return WHITE_QUEEN;
                case 'K': return WHITE_KING;
                case 'p': return BLACK_PAWN;
                case 'n': return BLACK_KNIGHT;
                case 'b': return BLACK_BISHOP;
                case 'r': return BLACK_ROOK;
                case 'q': return BLACK_QUEEN;
                case 'k': return BLACK_KING;
                default: return NO_PIECE;
            }
        }
    public:
        void loadFEN(const std::string& fen, BitboardInitializer& board){
            // Implementation to parse FEN and set up the board
            for(auto &bb :board.pieces){
                bb=0ULL;
            }
            int sq=56;
            for(char c:fen){
                if(c=='/'){
                    sq-=16;
                    continue;
                }
                if(isdigit(c)){
                    sq+=c-'0';
                    continue;
                
                }
                Piece piece=charToPiece(c);
                if(piece!=NO_PIECE){
                    board.pieces[piece]|=(1ULL<<sq);
                    
                }
                sq++;
                board.pieces[WHITE_PIECES]=0;
                board.pieces[BLACK_PIECES]=0;
                for(int i=WHITE_PAWN;i<=WHITE_KING;i++) board.pieces[WHITE_PIECES]|=board.pieces[i];
                for(int i=BLACK_PAWN;i<=BLACK_KING;i++) board.pieces[BLACK_PIECES]|=board.pieces[i];
                board.pieces[ALL_PIECES]=board.pieces[WHITE_PIECES]|board.pieces[BLACK_PIECES];
            }
        }
};

