#include <vector>
#include <string>
#include <iostream>
#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Helper.hpp"
#include "MoveGenerator.hpp"
#include "Position.hpp"
#include "magicBitboard.hpp"

using namespace std;
#include <map>

void printBoardWithPieces(BitboardInitializer &board) {
    // Map piece enum to symbols
    map<Piece, char> pieceSymbols = {
        {WHITE_PAWN, 'P'}, {WHITE_KNIGHT, 'N'}, {WHITE_BISHOP, 'B'},
        {WHITE_ROOK, 'R'}, {WHITE_QUEEN, 'Q'}, {WHITE_KING, 'K'},
        {BLACK_PAWN, 'p'}, {BLACK_KNIGHT, 'n'}, {BLACK_BISHOP, 'b'},
        {BLACK_ROOK, 'r'}, {BLACK_QUEEN, 'q'}, {BLACK_KING, 'k'}
    };

    cout << "  a b c d e f g h\n"; // File letters
    for (int rank = 7; rank >= 0; --rank) {
        cout << rank + 1 << " "; // Rank numbers
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            char symbol = '.';

            for (int p = WHITE_PAWN; p <= BLACK_KING; ++p) {
                if (board.pieces[p] & (1ULL << sq)) {
                    symbol = pieceSymbols[(Piece)p];
                    break;
                }
            }

            cout << symbol << " ";
        }
        cout << rank + 1 << "\n";
    }
    cout << "  a b c d e f g h\n";
}

void printMovesWithNames(const vector<pair<Piece, vector<pair<int,int>>>> &moves) {
    map<Piece, string> pieceNames = {
        {WHITE_PAWN, "White Pawn"}, {WHITE_KNIGHT, "White Knight"}, {WHITE_BISHOP, "White Bishop"},
        {WHITE_ROOK, "White Rook"}, {WHITE_QUEEN, "White Queen"}, {WHITE_KING, "White King"},
        {BLACK_PAWN, "Black Pawn"}, {BLACK_KNIGHT, "Black Knight"}, {BLACK_BISHOP, "Black Bishop"},
        {BLACK_ROOK, "Black Rook"}, {BLACK_QUEEN, "Black Queen"}, {BLACK_KING, "Black King"}
    };

    for (auto &entry : moves) {
        cout << entry.second.size() << " moves for " << pieceNames[entry.first] << ":\n";
        for (auto &mv : entry.second) {
            int fromRank = mv.first / 8 + 1;
            char fromFile = 'a' + (mv.first % 8);
            int toRank = mv.second / 8 + 1;
            char toFile = 'a' + (mv.second % 8);
            cout << "  " << fromFile << fromRank << " -> " << toFile << toRank << "\n";
        }
        cout << "\n";
    }
}

class Game {
public:
    BitboardInitializer board;
    MoveGen moveGen;
    Color sideToMove = WHITE;
    Helper fenHelper;

    Game() {
        InitAllAttackTables();
        InitMagicOnce();
    }

    void loadFEN(const string& fen) {
        string boardPart = fen.substr(0, fen.find(' '));
        string turnPart  = fen.substr(fen.find(' ') + 1, 1);

        fenHelper.loadFEN(boardPart, board);
        sideToMove = (turnPart == "w" ? WHITE : BLACK);
    }

    vector<pair<Piece, vector<pair<int,int>>>> 
    getAllMovesPerPiece() 
    {
        vector<pair<Piece, vector<pair<int,int>>>> result;

        Bitboard ourPieces = (sideToMove == WHITE) ? 
                              board.pieces[WHITE_PIECES] :
                              board.pieces[BLACK_PIECES];

        Bitboard oppPieces = (sideToMove == WHITE)
                              ? board.pieces[BLACK_PIECES]
                              : board.pieces[WHITE_PIECES];

        Bitboard occupancy = board.pieces[ALL_PIECES];

        Piece start = (sideToMove == WHITE) ? WHITE_PAWN : BLACK_PAWN;
        Piece end   = (sideToMove == WHITE) ? WHITE_KING : BLACK_KING;

        for (int p = start; p <= end; ++p) {
            Bitboard pieceBoard = board.pieces[p];
            vector<pair<int,int>> movesForPiece;

            while (pieceBoard) {
                int from = __builtin_ctzll(pieceBoard);
                pieceBoard &= pieceBoard - 1;

                Bitboard attacks = moveGen.GenerateAttacks(
                        (Square)from,
                        (Piece)p,
                        sideToMove,
                        &occupancy,
                        &ourPieces,
                        &oppPieces
                );

                // Remove friendly pieces (safety, in case generator missed)
                attacks &= ~ourPieces;

                while (attacks) {
                    int to = __builtin_ctzll(attacks);
                    attacks &= attacks - 1;
                    movesForPiece.push_back({ from, to });
                }
            }

            result.push_back({ (Piece)p, movesForPiece });
        }

        return result;
    }

    void printBoard() {
        cout << "Current Board Position:\n";
        board.printBitboard(board.pieces[ALL_PIECES]);
    }
};

int main() {
    Game g;
    g.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");

    auto moves = g.getAllMovesPerPiece();

    printBoardWithPieces(g.board);
printMovesWithNames(moves);

}
