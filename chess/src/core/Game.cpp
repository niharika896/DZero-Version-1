// game.cpp
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <cstring>
#include <cctype>
#include <optional>

#include "../core/PreComputedTables/PreComputed.hpp"
#include "BitBoard.hpp"
#include "Helper.hpp"
#include "MoveGenerator.hpp"
#include "Position.hpp"
#include "magicBitboard.hpp"

using namespace std;

// ---------------- Move struct ----------------
struct Move {
    int from;
    int to;
    Piece promotion = NO_PIECE; // NO_PIECE if no promotion
};

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
// Print human-readable moves (with promotions)
// ------------------------------------------------------
void printMovesWithNames(const vector<pair<Piece, vector<Move>>> &moves) {
    map<Piece, string> pieceNames = {
        {WHITE_PAWN, "White Pawn"}, {WHITE_KNIGHT, "White Knight"}, {WHITE_BISHOP, "White Bishop"},
        {WHITE_ROOK, "White Rook"}, {WHITE_QUEEN, "White Queen"}, {WHITE_KING, "White King"},
        {BLACK_PAWN, "Black Pawn"}, {BLACK_KNIGHT, "Black Knight"}, {BLACK_BISHOP, "Black Bishop"},
        {BLACK_ROOK, "Black Rook"}, {BLACK_QUEEN, "Black Queen"}, {BLACK_KING, "Black King"}
    };

    for (auto &entry : moves) {
        cout << entry.second.size() << " moves for " << pieceNames[entry.first] << ":\n";
        for (auto &mv : entry.second) {
            int from = mv.from;
            int to   = mv.to;

            char ff = 'a' + (from % 8);
            char tf = 'a' + (to   % 8);
            int fr  = from / 8 + 1;
            int tr  = to / 8 + 1;

            cout << "  " << ff << fr << " -> " << tf << tr;
            if (mv.promotion != NO_PIECE) {
                char pc = (mv.promotion==WHITE_QUEEN||mv.promotion==BLACK_QUEEN)?'Q':
                          (mv.promotion==WHITE_ROOK||mv.promotion==BLACK_ROOK)?'R':
                          (mv.promotion==WHITE_BISHOP||mv.promotion==BLACK_BISHOP)?'B':'N';
                cout << " (promote to " << pc << ")";
            }
            
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
// Utility functions
// ------------------------------------------------------
int squareFromStringFast(const std::string &s) {
    if (s.size() != 2) return -1;

    char file = toupper(s[0]);  // A–H
    char rank = s[1];           // 1–8

    if (file < 'A' || file > 'H') return -1;
    if (rank < '1' || rank > '8') return -1;

    int f = file - 'A';
    int r = rank - '1';

    return r * 8 + f;
}

Piece pieceFromFEN(char c) {
    switch (c) {
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

Piece promotionCharToPiece(char c, Color side) {
    c = tolower(c);
    if (side == WHITE) {
        if (c == 'q') return WHITE_QUEEN;
        if (c == 'r') return WHITE_ROOK;
        if (c == 'b') return WHITE_BISHOP;
        if (c == 'n') return WHITE_KNIGHT;
    } else {
        if (c == 'q') return BLACK_QUEEN;
        if (c == 'r') return BLACK_ROOK;
        if (c == 'b') return BLACK_BISHOP;
        if (c == 'n') return BLACK_KNIGHT;
    }
    return (side==WHITE?WHITE_QUEEN:BLACK_QUEEN); // default queen
}

// ------------------------------------------------------
// Game class (uses MoveGen)
// ------------------------------------------------------
class Game {
public:
    BitboardInitializer board;
    MoveGen moveGen;
    Helper fenHelper;
    Color sideToMove = WHITE;

    Game() {
        InitAllAttackTables();
        InitMagicOnce();
    }

    // Add this to your Game class
string getFEN(const GameState& state) {
    string fen = "";
    
    // 1. PIECE PLACEMENT (board part)
    for (int rank = 7; rank >= 0; rank--) {  // Start from rank 8 (index 7)
        int emptyCount = 0;
        
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            Piece piece = getPieceAt(state, sq);
            
            if (piece == NO_PIECE) {
                emptyCount++;
            } else {
                // If we had empty squares, add the count first
                if (emptyCount > 0) {
                    fen += to_string(emptyCount);
                    emptyCount = 0;
                }
                
                // Add piece character
                char pieceChar = pieceToChar(piece);
                fen += pieceChar;
            }
        }
        
        // Add remaining empty squares for this rank
        if (emptyCount > 0) {
            fen += to_string(emptyCount);
        }
        
        // Add rank separator (except after last rank)
        if (rank > 0) {
            fen += '/';
        }
    }
    
    // 2. SIDE TO MOVE
    fen += ' ';
    fen += (state.sideToMove == WHITE) ? 'w' : 'b';
    
    // 3. CASTLING RIGHTS
    fen += ' ';
    string castling = "";
    if (state.WhiteCanCastleKingSide) castling += 'K';
    if (state.WhiteCanCastleQueenSide) castling += 'Q';
    if (state.BlackCanCastleKingSide) castling += 'k';
    if (state.BlackCanCastleQueenSide) castling += 'q';
    if (castling.empty()) castling = "-";
    fen += castling;
    
    // 4. EN PASSANT TARGET SQUARE
    fen += ' ';
    if (state.enPassantSquare == -1) {
        fen += '-';
    } else {
        fen += squareToStringINT(state.enPassantSquare);
    }
    
    // 5. HALFMOVE CLOCK (for 50-move rule, defaulting to 0)
    fen += " 0";
    
    // 6. FULLMOVE NUMBER (defaulting to 1)
    fen += " 1";
    
    return fen;
}

// Helper function to convert Piece enum to FEN character
char pieceToChar(Piece p) {
    switch(p) {
        case WHITE_PAWN:   return 'P';
        case WHITE_KNIGHT: return 'N';
        case WHITE_BISHOP: return 'B';
        case WHITE_ROOK:   return 'R';
        case WHITE_QUEEN:  return 'Q';
        case WHITE_KING:   return 'K';
        case BLACK_PAWN:   return 'p';
        case BLACK_KNIGHT: return 'n';
        case BLACK_BISHOP: return 'b';
        case BLACK_ROOK:   return 'r';
        case BLACK_QUEEN:  return 'q';
        case BLACK_KING:   return 'k';
        default: return '?';
    }
}

    void loadFEN(const string &fen) {
        string boardPart = fen.substr(0, fen.find(' '));
        string turnPart  = fen.substr(fen.find(' ') + 1, 1);

        fenHelper.loadFEN(boardPart, board);
        sideToMove = (turnPart == "w" ? WHITE : BLACK);
    }

    // find first king square for given color (returns -1 if none)
    int findKing(const GameState &s, Color c) {
        Bitboard king = (c == WHITE) ? s.board.pieces[WHITE_KING] : s.board.pieces[BLACK_KING];
        if (!king) return -1;
        return __builtin_ctzll(king);
    }

    // get piece on square (NO_PIECE if empty)
    Piece getPieceAt(const GameState &s, int sq) {
        if (sq < 0 || sq >= 64) return NO_PIECE;
        for (int p = WHITE_PAWN; p <= BLACK_KING; ++p) {
            if (s.board.pieces[p] & (1ULL << sq)) return (Piece)p;
        }
        return NO_PIECE;
    }

    // is square 'sq' attacked by color 'attacker' in given GameState s ?
    bool isSquareAttacked(GameState &s, Color attacker, int sq) {
        if (sq < 0 || sq > 63) return false;

        Bitboard occupancy = s.board.pieces[ALL_PIECES];
        const Bitboard* wptr = &s.board.pieces[WHITE_PIECES];
        const Bitboard* bptr = &s.board.pieces[BLACK_PIECES];

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
                    wptr,
                    bptr,
                    s.enPassantSquare
                );
                if (attacks & (1ULL << sq)) return true;
            }
        }

        return false;
    }

    // Check if side 'c' is currently in check in GameState s
    bool isInCheck(GameState &s, Color c) {
        int kingSq = findKing(s, c);
        if (kingSq == -1) return false;
        Color attacker = (c == WHITE) ? BLACK : WHITE;
        return isSquareAttacked(s, attacker, kingSq);
    }

    // Make move (handles promotions, castling, en-passant, castling-rights update)
    void makeMove(GameState &s, const Move &m) {
        int from = m.from;
        int to   = m.to;
        Piece mover = getPieceAt(s, from);
        if (mover == NO_PIECE) return;

        Bitboard fromBB = (1ULL << from);
        Bitboard toBB   = (1ULL << to);

        // remove mover from source
        s.board.pieces[mover] &= ~fromBB;

        // remove capture on destination if any
        for (int pc = WHITE_PAWN; pc <= BLACK_KING; ++pc) {
            if (s.board.pieces[pc] & toBB) s.board.pieces[pc] &= ~toBB;
        }

        // en-passant capture
        if ((mover == WHITE_PAWN || mover == BLACK_PAWN) && s.enPassantSquare != -1 && to == s.enPassantSquare) {
            if (mover == WHITE_PAWN) {
                int capSq = to - 8;
                if (capSq >= 0) s.board.pieces[BLACK_PAWN] &= ~(1ULL << capSq);
            } else {
                int capSq = to + 8;
                if (capSq < 64) s.board.pieces[WHITE_PAWN] &= ~(1ULL << capSq);
            }
        }

        // handle castling movement if king moved from e-file
        bool handledCastle = false;
        if (mover == WHITE_KING) {
            if (from == 4 && to == 6 && s.WhiteCanCastleKingSide) {
                // e1->g1 ; rook h1->f1
                s.board.pieces[WHITE_KING] |= (1ULL << 6);
                s.board.pieces[WHITE_ROOK] &= ~(1ULL << 7);
                s.board.pieces[WHITE_ROOK] |=  (1ULL << 5);
                s.WhiteCanCastleKingSide = s.WhiteCanCastleQueenSide = 0;
                handledCastle = true;
            } else if (from == 4 && to == 2 && s.WhiteCanCastleQueenSide) {
                // e1->c1 ; rook a1->d1
                s.board.pieces[WHITE_KING] |= (1ULL << 2);
                s.board.pieces[WHITE_ROOK] &= ~(1ULL << 0);
                s.board.pieces[WHITE_ROOK] |=  (1ULL << 3);
                s.WhiteCanCastleKingSide = s.WhiteCanCastleQueenSide = 0;
                handledCastle = true;
            }
        } else if (mover == BLACK_KING) {
            if (from == 60 && to == 62 && s.BlackCanCastleKingSide) {
                s.board.pieces[BLACK_KING] |= (1ULL << 62);
                s.board.pieces[BLACK_ROOK] &= ~(1ULL << 63);
                s.board.pieces[BLACK_ROOK] |=  (1ULL << 61);
                s.BlackCanCastleKingSide = s.BlackCanCastleQueenSide = 0;
                handledCastle = true;
            } else if (from == 60 && to == 58 && s.BlackCanCastleQueenSide) {
                s.board.pieces[BLACK_KING] |= (1ULL << 58);
                s.board.pieces[BLACK_ROOK] &= ~(1ULL << 56);
                s.board.pieces[BLACK_ROOK] |=  (1ULL << 59);
                s.BlackCanCastleKingSide = s.BlackCanCastleQueenSide = 0;
                handledCastle = true;
            }
        }

        if (!handledCastle) {
            // promotions
            if ((mover == WHITE_PAWN || mover == BLACK_PAWN) && m.promotion != NO_PIECE) {
                // ensure pawn removed from source already; place promoted piece on to
                s.board.pieces[m.promotion] |= toBB;
            } else {
                // normal placement
                s.board.pieces[mover] |= toBB;
            }

            // update castling rights
            if (mover == WHITE_KING) {
                s.WhiteCanCastleKingSide = s.WhiteCanCastleQueenSide = 0;
            }
            if (mover == BLACK_KING) {
                s.BlackCanCastleKingSide = s.BlackCanCastleQueenSide = 0;
            }
            if (mover == WHITE_ROOK) {
                if (from == 0) s.WhiteCanCastleQueenSide = 0;
                if (from == 7) s.WhiteCanCastleKingSide = 0;
            }
            if (mover == BLACK_ROOK) {
                if (from == 56) s.BlackCanCastleQueenSide = 0;
                if (from == 63) s.BlackCanCastleKingSide = 0;
            }
            // if a rook was captured on its initial square, clear rights
            if (to == 0) s.WhiteCanCastleQueenSide = 0;
            if (to == 7) s.WhiteCanCastleKingSide = 0;
            if (to == 56) s.BlackCanCastleQueenSide = 0;
            if (to == 63) s.BlackCanCastleKingSide = 0;

            // update en-passant target
            s.enPassantSquare = -1;
            if (mover == WHITE_PAWN) {
                int fromRank = from / 8;
                int toRank = to / 8;
                if (fromRank == 1 && toRank == 3) s.enPassantSquare = from + 8;
            } else if (mover == BLACK_PAWN) {
                int fromRank = from / 8;
                int toRank = to / 8;
                if (fromRank == 6 && toRank == 4) s.enPassantSquare = from - 8;
            }
        } else {
            // castle cleared en-passant
            s.enPassantSquare = -1;
        }

        updateCombinedBoards(s);
    }

    // Generate pseudo-legal moves (produces promotions as separate Move entries and castling)
    vector<pair<Piece, vector<Move>>> getAllMovesPerPiece(GameState &state) {
        vector<pair<Piece, vector<Move>>> result;

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
            vector<Move> movesForPiece;

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

                    // Pawn promotion handling: generate 4 promotion options when target rank is final rank
                    if (p == (state.sideToMove == WHITE ? WHITE_PAWN : BLACK_PAWN)) {
                        int toRank = to / 8;
                        bool isPromo = (state.sideToMove == WHITE) ? (toRank == 7) : (toRank == 0);
                        if (isPromo) {
                            // push all promotions
                            Move mQ{from, to, (state.sideToMove==WHITE?WHITE_QUEEN:BLACK_QUEEN)};
                            Move mR{from, to, (state.sideToMove==WHITE?WHITE_ROOK:BLACK_ROOK)};
                            Move mB{from, to, (state.sideToMove==WHITE?WHITE_BISHOP:BLACK_BISHOP)};
                            Move mN{from, to, (state.sideToMove==WHITE?WHITE_KNIGHT:BLACK_KNIGHT)};
                            movesForPiece.push_back(mQ);
                            movesForPiece.push_back(mR);
                            movesForPiece.push_back(mB);
                            movesForPiece.push_back(mN);
                            continue;
                        }
                    }

                    movesForPiece.push_back(Move{from, to, NO_PIECE});
                }

                // Castling: if this piece is the king add castling pseudo-moves if rights and empty and not attacked (we still legalize later)
                if (p == (state.sideToMove == WHITE ? WHITE_KING : BLACK_KING)) {
                    if (state.sideToMove == WHITE) {
                        // white king-side e1->g1 (4->6)
                        if (state.WhiteCanCastleKingSide) {
                            if (!(occupancy & ((1ULL<<5)|(1ULL<<6)))) {
                                // check squares e1,f1,g1 not attacked
                                bool eSafe = !isSquareAttacked(const_cast<GameState&>(state), BLACK, 4);
                                bool fSafe = !isSquareAttacked(const_cast<GameState&>(state), BLACK, 5);
                                bool gSafe = !isSquareAttacked(const_cast<GameState&>(state), BLACK, 6);
                                if (eSafe && fSafe && gSafe) movesForPiece.push_back(Move{4,6,NO_PIECE});
                            }
                        }
                        // white queen-side e1->c1 (4->2)
                        if (state.WhiteCanCastleQueenSide) {
                            if (!(occupancy & ((1ULL<<1)|(1ULL<<2)|(1ULL<<3)))) {
                                bool eSafe = !isSquareAttacked(const_cast<GameState&>(state), BLACK, 4);
                                bool dSafe = !isSquareAttacked(const_cast<GameState&>(state), BLACK, 3);
                                bool cSafe = !isSquareAttacked(const_cast<GameState&>(state), BLACK, 2);
                                if (eSafe && dSafe && cSafe) movesForPiece.push_back(Move{4,2,NO_PIECE});
                            }
                        }
                    } else {
                        // black king-side e8->g8 (60->62)
                        if (state.BlackCanCastleKingSide) {
                            if (!(occupancy & ((1ULL<<61)|(1ULL<<62)))) {
                                bool eSafe = !isSquareAttacked(const_cast<GameState&>(state), WHITE, 60);
                                bool fSafe = !isSquareAttacked(const_cast<GameState&>(state), WHITE, 61);
                                bool gSafe = !isSquareAttacked(const_cast<GameState&>(state), WHITE, 62);
                                if (eSafe && fSafe && gSafe) movesForPiece.push_back(Move{60,62,NO_PIECE});
                            }
                        }
                        // black queen-side e8->c8 (60->58)
                        if (state.BlackCanCastleQueenSide) {
                            if (!(occupancy & ((1ULL<<57)|(1ULL<<58)|(1ULL<<59)))) {
                                bool eSafe = !isSquareAttacked(const_cast<GameState&>(state), WHITE, 60);
                                bool dSafe = !isSquareAttacked(const_cast<GameState&>(state), WHITE, 59);
                                bool cSafe = !isSquareAttacked(const_cast<GameState&>(state), WHITE, 58);
                                if (eSafe && dSafe && cSafe) movesForPiece.push_back(Move{60,58,NO_PIECE});
                            }
                        }
                    }
                }
            }

            result.push_back({(Piece)p, movesForPiece});
        }

        return result;
    }

    // Legalize pseudo-legal moves: return same structure but only legal ones
    vector<pair<Piece, vector<Move>>> getAllLegalMoves(GameState &state) {
        vector<pair<Piece, vector<Move>>> pseudo = getAllMovesPerPiece(state);
        vector<pair<Piece, vector<Move>>> legal;

        Color moverSide = state.sideToMove;
        Color opponent = (moverSide == WHITE) ? BLACK : WHITE;

        for (auto &entry : pseudo) {
            Piece pieceType = entry.first;
            vector<Move> legalForThisPiece;

            for (auto &mv : entry.second) {
                GameState copy = state;
                makeMove(copy, mv);

                // flip side
                copy.sideToMove = opponent;

                int kingSq = findKing(copy, moverSide);
                if (kingSq == -1) {
                    continue;
                }

                // if opponent attacks the mover's king in the resulting position, move is illegal
                if (!isSquareAttacked(copy, opponent, kingSq)) {
                    legalForThisPiece.push_back(mv);
                }
            }

            legal.push_back({pieceType, legalForThisPiece});
        }

        return legal;
    }
};
int evaluateBoard(Game &g, GameState &state, bool isMaximisingPlayer) {
    auto legal = g.getAllLegalMoves(state);
    int totalMoves = 0;
    for (auto &e : legal) totalMoves += e.second.size();
    bool inCheck = g.isInCheck(state, state.sideToMove);

    // compute maximizing player's color from parameters
    Color maxColor = isMaximisingPlayer ? state.sideToMove
                                        : (state.sideToMove == WHITE ? BLACK : WHITE);

    if (totalMoves == 0) {
        if (inCheck) {
            Color winner = (state.sideToMove == WHITE) ? BLACK : WHITE;
            return (winner == maxColor) ? 100000000 : -100000000;
        }
        return 0; // stalemate
    }
    Color botColor    = isMaximisingPlayer ? state.sideToMove : (state.sideToMove==WHITE?BLACK:WHITE);
    Color enemyColor  = (botColor == WHITE ? BLACK : WHITE);

    auto count = [&](Piece p) {
        return __builtin_popcountll(state.board.pieces[p]);
    };

    int botMaterial =
          count(botColor==WHITE ? WHITE_QUEEN  : BLACK_QUEEN ) * 900
        + count(botColor==WHITE ? WHITE_ROOK   : BLACK_ROOK  ) * 500
        + count(botColor==WHITE ? WHITE_BISHOP : BLACK_BISHOP) * 330
        + count(botColor==WHITE ? WHITE_KNIGHT : BLACK_KNIGHT) * 320
        + count(botColor==WHITE ? WHITE_PAWN   : BLACK_PAWN  ) * 100;

    int enemyMaterial =
          count(enemyColor==WHITE ? WHITE_QUEEN  : BLACK_QUEEN ) * 900
        + count(enemyColor==WHITE ? WHITE_ROOK   : BLACK_ROOK  ) * 500
        + count(enemyColor==WHITE ? WHITE_BISHOP : BLACK_BISHOP) * 330
        + count(enemyColor==WHITE ? WHITE_KNIGHT : BLACK_KNIGHT) * 320
        + count(enemyColor==WHITE ? WHITE_PAWN   : BLACK_PAWN  ) * 100;

    return botMaterial - enemyMaterial;
    
}

int minimax(Game &g, GameState &state, int depth,int alpha,int beta, bool isMaximisingPlayer) {
    if (depth == 0)
        return evaluateBoard(g, state, isMaximisingPlayer);

    auto legal = g.getAllLegalMoves(state);

    int totalMoves = 0;
    for (auto &e : legal) totalMoves += e.second.size();
    if (totalMoves == 0)
        return evaluateBoard(g, state, isMaximisingPlayer);

    if (isMaximisingPlayer) {
        int bestEval = -100000000;

        for (auto &entry : legal) {
            for (auto &mv : entry.second) {

                GameState backup = state;        // SAVE
                g.makeMove(state, mv);           // APPLY MOVE
                updateCombinedBoards(state);

                state.sideToMove = (state.sideToMove==WHITE?BLACK:WHITE);

                int eval = minimax(g, state, depth - 1,alpha,beta, false);
                alpha = max(alpha,eval);
                state = backup;                  // RESTORE

                bestEval = max(bestEval, eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }

        return bestEval;
    }

    else {
        int bestEval = 100000000;

        for (auto &entry : legal) {
            for (auto &mv : entry.second) {

                GameState backup = state; 

                g.makeMove(state, mv);
                updateCombinedBoards(state);

                state.sideToMove = (state.sideToMove==WHITE?BLACK:WHITE);

                int eval = minimax(g, state, depth - 1,alpha,beta, true);

                state = backup;

                bestEval = min(bestEval, eval);
                beta = min(beta,eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }

        return bestEval;
    }
}

// ------------------------------------------------------
// MAIN
// ------------------------------------------------------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cerr << "C++ ENGINE STARTED" << endl;

    while (true) {
        string fen;
        string fromS, toS;

        cerr << "WAITING FOR INPUT..." << endl;
        
        // READ INPUT
        if (!getline(cin, fen)) {
            cerr << "FEN read failed, exiting" << endl;
            break;
        }
        cerr << "GOT FEN: " << fen << endl;
        
        if (!(cin >> fromS >> toS)) {
            cerr << "MOVE read failed, exiting" << endl;
            break;
        }
        cin.ignore();  // Clear newline
        cerr << "GOT MOVE: " << fromS << " " << toS << endl;

        // INIT GAME
        Game g;
        g.loadFEN(fen);

        GameState state;
        state.board = g.board;
        state.sideToMove = g.sideToMove;
        state.enPassantSquare = -1;
        
        state.WhiteCanCastleKingSide = 1;
        state.WhiteCanCastleQueenSide = 1;
        state.BlackCanCastleKingSide = 1;
        state.BlackCanCastleQueenSide = 1;

        updateCombinedBoards(state);

        // APPLY HUMAN MOVE
        if (fromS != "NONE" && toS != "NONE") {
            int from = squareFromStringFast(fromS);
            int to   = squareFromStringFast(toS);

            auto legal = g.getAllLegalMoves(state);
            vector<Move> candidates;

            for (auto &entry : legal)
                for (auto &mv : entry.second)
                    if (mv.from == from && mv.to == to)
                        candidates.push_back(mv);

            if (!candidates.empty()) {
                g.makeMove(state, candidates[0]);
                updateCombinedBoards(state);
                state.sideToMove = BLACK;
            }
        }

        // BOT MOVE
        auto botLegal = g.getAllLegalMoves(state);

        int bestEval = 100000000;
        Move bestMove;
        bool moveSet = false;

        for (auto &entry : botLegal) {
            for (auto &mv : entry.second) {
                GameState backup = state;

                g.makeMove(state, mv);
                updateCombinedBoards(state);
                state.sideToMove = WHITE;

                int eval = minimax(g, state, 2, -100000000, 100000000, true);

                state = backup;

                if (!moveSet || eval < bestEval) {
                    bestEval = eval;
                    bestMove = mv;
                    moveSet = true;
                }
            }
        }

        // MAKE BOT MOVE
        g.makeMove(state, bestMove);
        updateCombinedBoards(state);
        state.sideToMove = WHITE;

        // OUTPUT BOTH MOVE AND FEN
        string outFrom = squareToStringINT(bestMove.from);
        string outTo   = squareToStringINT(bestMove.to);

        // Line 1: MOVE
        cout << outFrom << " " << outTo;
        if (bestMove.promotion != NO_PIECE) {
            char promo = '?';
            if (bestMove.promotion == BLACK_QUEEN)  promo = 'Q';
            if (bestMove.promotion == BLACK_ROOK)   promo = 'R';
            if (bestMove.promotion == BLACK_BISHOP) promo = 'B';
            if (bestMove.promotion == BLACK_KNIGHT) promo = 'N';
            cout << " " << promo;
        }
        cout << endl;
        cout.flush();  // IMPORTANT: Flush after move

        // Line 2: FEN
        string resultFEN = g.getFEN(state);
        cout << resultFEN << endl;
        cout.flush();  // IMPORTANT: Flush after FEN
        
        cerr << "OUTPUT COMPLETE, looping back..." << endl;
    }

    cerr << "C++ ENGINE EXITING" << endl;
    return 0;
}