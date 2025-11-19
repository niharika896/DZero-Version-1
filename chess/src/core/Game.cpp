#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <cstring>

#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Helper.hpp"
#include "MoveGenerator.hpp"
#include "Position.hpp"
#include "magicBitboard.hpp"

using namespace std;

// ------------------------------------------------------
// GameState structure (en passant included)
// ------------------------------------------------------
struct GameState {
    Color sideToMove;
    BitboardInitializer board;

    int WhitePawn[8] = {1,1,1,1,1,1,1,1};
    int BlackPawn[8] = {1,1,1,1,1,1,1,1};

    int WhiteCanCastleKingSide = 1;
    int WhiteCanCastleQueenSide = 1;
    int BlackCanCastleKingSide = 1;
    int BlackCanCastleQueenSide = 1;

    int enPassantSquare = -1; // default: none
};

// ------------------------------------------------------
// Pretty-print board (with ASCII pieces)
// ------------------------------------------------------
void printBoardWithPieces(const GameState &state) {
    map<Piece, char> pieceSymbols = {
        {WHITE_PAWN, 'P'}, {WHITE_KNIGHT, 'N'}, {WHITE_BISHOP, 'B'},
        {WHITE_ROOK, 'R'}, {WHITE_QUEEN, 'Q'}, {WHITE_KING, 'K'},
        {BLACK_PAWN, 'p'}, {BLACK_KNIGHT, 'n'}, {BLACK_BISHOP, 'b'},
        {BLACK_ROOK, 'r'}, {BLACK_QUEEN, 'q'}, {BLACK_KING, 'k'}
    };

    cout << "  a b c d e f g h\n";
    for (int rank = 7; rank >= 0; --rank) {
        cout << rank + 1 << " ";
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            char symbol = '.';

            for (int p = WHITE_PAWN; p <= BLACK_KING; ++p) {
                if (state.board.pieces[p] & (1ULL << sq)) {
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

// ------------------------------------------------------
// Print human-readable moves
// ------------------------------------------------------
void printMovesWithNames(const vector<pair<Piece, vector<pair<int,int>>>> &moves) {
    map<Piece, string> pieceNames = {
        {WHITE_PAWN, "White Pawn"}, {WHITE_KNIGHT, "White Knight"},
        {WHITE_BISHOP, "White Bishop"}, {WHITE_ROOK, "White Rook"},
        {WHITE_QUEEN, "White Queen"}, {WHITE_KING, "White King"},
        {BLACK_PAWN, "Black Pawn"}, {BLACK_KNIGHT, "Black Knight"},
        {BLACK_BISHOP, "Black Bishop"}, {BLACK_ROOK, "Black Rook"},
        {BLACK_QUEEN, "Black Queen"}, {BLACK_KING, "Black King"}
    };

    for (auto &entry : moves) {
        cout << entry.second.size() << " moves for " << pieceNames[entry.first] << ":\n";
        for (auto &mv : entry.second) {
            int from = mv.first;
            int to   = mv.second;

            char ff = 'a' + (from % 8);
            char tf = 'a' + (to   % 8);
            int fr  = from / 8 + 1;
            int tr  = to   / 8 + 1;

            cout << "  " << ff << fr << " -> " << tf << tr << "\n";
        }
        cout << "\n";
    }
}

// ------------------------------------------------------
// Small helpers to update combined bitboards
// ------------------------------------------------------
void updateCombinedBoards(GameState &s) {
    Bitboard whiteAll = 0ULL;
    Bitboard blackAll = 0ULL;

    for (int p = WHITE_PAWN; p <= WHITE_KING; ++p) whiteAll |= s.board.pieces[p];
    for (int p = BLACK_PAWN; p <= BLACK_KING; ++p) blackAll |= s.board.pieces[p];

    s.board.pieces[WHITE_PIECES] = whiteAll;
    s.board.pieces[BLACK_PIECES] = blackAll;
    s.board.pieces[ALL_PIECES]   = whiteAll | blackAll;
}

// ------------------------------------------------------
// Game - uses new MoveGen with correct pawn generation
// ------------------------------------------------------
class Game {
public:
    BitboardInitializer board;
    MoveGen moveGen;
    Helper fenHelper;
    Color sideToMove = WHITE;

    int enPassantSq = -1;

    Game() {
        InitAllAttackTables();
        InitMagicOnce();
    }

    void loadFEN(const string &fen) {
        string boardPart = fen.substr(0, fen.find(' '));
        string turnPart  = fen.substr(fen.find(' ') + 1, 1);

        fenHelper.loadFEN(boardPart, board);
        sideToMove = (turnPart == "w" ? WHITE : BLACK);

        enPassantSq = -1;
    }

    // find first king square for given color (returns -1 if none)
    int findKing(const GameState &s, Color c) {
        Bitboard king = (c == WHITE) ? s.board.pieces[WHITE_KING] : s.board.pieces[BLACK_KING];
        if (!king) return -1;
        return __builtin_ctzll(king);
    }

    // Make a move on the GameState (simple version).
    // from/to are square indices 0..63, mover is the piece being moved (WHITE_PAWN .. BLACK_KING).
    // side is the color moving (WHITE or BLACK).
    void makeMove(GameState &s, int from, int to, Piece mover, Color side) {
        Bitboard fromBB = (1ULL << from);
        Bitboard toBB   = (1ULL << to);

        // remove mover from source
        s.board.pieces[mover] &= ~fromBB;

        // detect capture: remove any piece present on 'to'
        for (int pc = WHITE_PAWN; pc <= BLACK_KING; ++pc) {
            if (s.board.pieces[pc] & toBB) {
                s.board.pieces[pc] &= ~toBB;
            }
        }

        // Handle en-passant capture: if mover is pawn and moves to enPassantSquare,
        // remove the pawn that moved two squares in previous move (the pawn sits behind the en-passant square).
        if ((mover == WHITE_PAWN || mover == BLACK_PAWN) && s.enPassantSquare != -1 && to == s.enPassantSquare) {
            if (side == WHITE) {
                // white captured black pawn that was on to - 8
                int capSq = to - 8;
                if (capSq >= 0 && capSq < 64) {
                    Bitboard capBB = (1ULL << capSq);
                    // remove black pawn at capSq
                    s.board.pieces[BLACK_PAWN] &= ~capBB;
                }
            } else {
                // black captured white pawn that was on to + 8
                int capSq = to + 8;
                if (capSq >= 0 && capSq < 64) {
                    Bitboard capBB = (1ULL << capSq);
                    s.board.pieces[WHITE_PAWN] &= ~capBB;
                }
            }
        }

        // place mover on destination
        s.board.pieces[mover] |= toBB;

        // update enPassant square for next side: if pawn moved two squares, set passed-over square
        s.enPassantSquare = -1;
        if (mover == WHITE_PAWN) {
            int fromRank = from / 8;
            int toRank = to / 8;
            if (fromRank == 1 && toRank == 3) { // white moved two squares 1->3
                s.enPassantSquare = from + 8; // square passed over
            }
        } else if (mover == BLACK_PAWN) {
            int fromRank = from / 8;
            int toRank = to / 8;
            if (fromRank == 6 && toRank == 4) { // black moved two squares 6->4
                s.enPassantSquare = from - 8; // square passed over
            }
        }

        // recompute combined boards
        updateCombinedBoards(s);
    }

    // Is square 'sq' attacked by color 'attacker' in given GameState s ?
    bool isSquareAttacked(GameState &s, Color attacker, int sq) {
        if (sq < 0 || sq > 63) return false;

        Bitboard occupancy = s.board.pieces[ALL_PIECES];
        Bitboard ourPieces   = (attacker == WHITE) ? s.board.pieces[WHITE_PIECES] : s.board.pieces[BLACK_PIECES];
        Bitboard enemyPieces = (attacker == WHITE) ? s.board.pieces[BLACK_PIECES] : s.board.pieces[WHITE_PIECES];

        // Iterate over every piece of attacker and see if any attack includes 'sq'
        Piece start = (attacker == WHITE) ? WHITE_PAWN : BLACK_PAWN;
        Piece end   = (attacker == WHITE) ? WHITE_KING : BLACK_KING;

        for (int p = start; p <= end; ++p) {
            Bitboard bb = s.board.pieces[p];
            while (bb) {
                int from = __builtin_ctzll(bb);
                bb &= bb - 1;

                Bitboard attacks = moveGen.GenerateAttacks(
                    (Square)from,
                    (Piece)p,
                    attacker,
                    &occupancy,
                    &s.board.pieces[WHITE_PIECES],
                    &s.board.pieces[BLACK_PIECES],
                    s.enPassantSquare
                );

                if (attacks & (1ULL << sq)) return true;
            }
        }

        return false;
    }

    // Generate pseudo-legal moves (same as before)
    vector<pair<Piece, vector<pair<int,int>>>> getAllMovesPerPiece(GameState &state) {
        vector<pair<Piece, vector<pair<int,int>>>> result;

        Bitboard ourPieces = (state.sideToMove == WHITE)
                             ? state.board.pieces[WHITE_PIECES]
                             : state.board.pieces[BLACK_PIECES];

        Bitboard oppPieces = (state.sideToMove == WHITE)
                             ? state.board.pieces[BLACK_PIECES]
                             : state.board.pieces[WHITE_PIECES];

        Bitboard occupancy = state.board.pieces[ALL_PIECES];

        Piece start = (state.sideToMove == WHITE ? WHITE_PAWN : BLACK_PAWN);
        Piece end   = (state.sideToMove == WHITE ? WHITE_KING : BLACK_KING);

        for (int p = start; p <= end; ++p) {
            Bitboard bb = state.board.pieces[p];
            vector<pair<int,int>> movesForPiece;

            while (bb) {
                int from = __builtin_ctzll(bb);
                bb &= bb - 1;

                Bitboard moves = moveGen.GenerateAttacks(
                        (Square)from,
                        (Piece)p,
                        state.sideToMove,
                        &occupancy,
                        &state.board.pieces[WHITE_PIECES],
                        &state.board.pieces[BLACK_PIECES],
                        state.enPassantSquare
                );

                // Remove friendly (safety)
                moves &= ~ourPieces;

                while (moves) {
                    int to = __builtin_ctzll(moves);
                    moves &= moves - 1;
                    movesForPiece.push_back({from, to});
                }
            }

            result.push_back({(Piece)p, movesForPiece});
        }

        return result;
    }

    // Legalize pseudo-legal moves: return same structure but only legal ones
    vector<pair<Piece, vector<pair<int,int>>>> getAllLegalMoves(GameState &state) {
        vector<pair<Piece, vector<pair<int,int>>>> pseudo = getAllMovesPerPiece(state);
        vector<pair<Piece, vector<pair<int,int>>>> legal;

        Color moverSide = state.sideToMove;
        Color opponent = (moverSide == WHITE) ? BLACK : WHITE;

        for (auto &entry : pseudo) {
            Piece pieceType = entry.first;
            vector<pair<int,int>> legalForThisPiece;

            for (auto &mv : entry.second) {
                int from = mv.first;
                int to   = mv.second;

                // copy state and make the move
                GameState copy = state;
                makeMove(copy, from, to, pieceType, moverSide);

                // side flips after move
                copy.sideToMove = opponent;

                // find mover's king square in the new position
                int kingSq = findKing(copy, moverSide);
                if (kingSq == -1) {
                    // no king found - treat as illegal
                    continue;
                }

                // check if opponent attacks mover's king
                bool attacked = isSquareAttacked(copy, opponent, kingSq);

                if (!attacked) {
                    // move is legal
                    legalForThisPiece.push_back(mv);
                }
            }

            legal.push_back({pieceType, legalForThisPiece});
        }

        return legal;
    }
};

// ------------------------------------------------------
// MAIN
// ------------------------------------------------------
int main() {
    Game g;
    g.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");

    // Build initial state wrapper
    GameState state;
    state.board = g.board;
    state.sideToMove = g.sideToMove;
    state.enPassantSquare = -1;

    // Update combined boards to be safe
    updateCombinedBoards(state);

    // Get pseudo-legal moves
    auto pseudo = g.getAllMovesPerPiece(state);
    cout << "Pseudo-legal moves:\n";
    printMovesWithNames(pseudo);

    // Get legal moves (after making each move and checking king safety)
    auto legal = g.getAllLegalMoves(state);
    cout << "Legal moves (after legalization):\n";
    printMovesWithNames(legal);

    // Example: print whether sideToMove's king would be in check after each legal move
    Color mover = state.sideToMove;
    Color opp = (mover == WHITE) ? BLACK : WHITE;
    return 0;
}
