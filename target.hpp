#include <algorithm>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BitboardNames.hpp"
#include "BitboardOps.hpp"
#include "ConstexprArrayGenerator.hpp"
#include "HashCounter.hpp"
#include "Index.hpp"
#include "SquareIterator.hpp"
#include "SquareSet.hpp"
#include "StringTools.hpp"
#include "scan.hpp"

namespace Chess {

using namespace std::literals;

enum class _EnPassantSpec {
    legal,
    fen,
    xfen
};

enum Color : bool {
    WHITE,
    BLACK
};

const std::string COLOR_NAMES[] = { "black", "white" };

enum class PieceType {
    PAWN = 1,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

const std::array<std::string, 7> PIECE_SYMBOLS = { "-", "p", "n", "b", "r", "q", "k" };
const std::array<std::string, 7> PIECE_NAMES = { "-", "pawn", "knight", "bishop", "rook", "queen", "king" };

auto piece_symbol(PieceType piece_type) -> std::string {
    return PIECE_SYMBOLS[(size_t)piece_type];
}

auto piece_name(PieceType piece_type) -> std::string {
    return PIECE_NAMES[(size_t)piece_type];
}

const std::map<std::string, std::string> UNICODE_PIECE_SYMBOLS = {
    {"R", "♖"}, {"r", "♜"},
    {"N", "♘"}, {"n", "♞"},
    {"B", "♗"}, {"b", "♝"},
    {"Q", "♕"}, {"q", "♛"},
    {"K", "♔"}, {"k", "♚"},
    {"P", "♙"}, {"p", "♟"},
};

constexpr std::array<char, 8> FILE_NAMES = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

constexpr std::array<char, 8> RANK_NAMES = {'1', '2', '3', '4', '5', '6', '7', '8'};

const std::string STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
// The FEN for the standard chess starting position.

const std::string STARTING_BOARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
// The board part of the FEN for the standard chess starting position.

enum class Status : int {
    VALID = 0,
    NO_WHITE_KING = 1 << 0,
    NO_BLACK_KING = 1 << 1,
    TOO_MANY_KINGS = 1 << 2,
    TOO_MANY_WHITE_PAWNS = 1 << 3,
    TOO_MANY_BLACK_PAWNS = 1 << 4,
    PAWNS_ON_BACKRANK = 1 << 5,
    TOO_MANY_WHITE_PIECES = 1 << 6,
    TOO_MANY_BLACK_PIECES = 1 << 7,
    BAD_CASTLING_RIGHTS = 1 << 8,
    INVALID_EP_SQUARE = 1 << 9,
    OPPOSITE_CHECK = 1 << 10,
    EMPTY = 1 << 11,
    RACE_CHECK = 1 << 12,
    RACE_OVER = 1 << 13,
    RACE_MATERIAL = 1 << 14,
    TOO_MANY_CHECKERS = 1 << 15,
    IMPOSSIBLE_CHECK = 1 << 16,
};

constexpr auto STATUS_VALID = Status::VALID;
constexpr auto STATUS_NO_WHITE_KING = Status::NO_WHITE_KING;
constexpr auto STATUS_NO_BLACK_KING = Status::NO_BLACK_KING;
constexpr auto STATUS_TOO_MANY_KINGS = Status::TOO_MANY_KINGS;
constexpr auto STATUS_TOO_MANY_WHITE_PAWNS = Status::TOO_MANY_WHITE_PAWNS;
constexpr auto STATUS_TOO_MANY_BLACK_PAWNS = Status::TOO_MANY_BLACK_PAWNS;
constexpr auto STATUS_PAWNS_ON_BACKRANK = Status::PAWNS_ON_BACKRANK;
constexpr auto STATUS_TOO_MANY_WHITE_PIECES = Status::TOO_MANY_WHITE_PIECES;
constexpr auto STATUS_TOO_MANY_BLACK_PIECES = Status::TOO_MANY_BLACK_PIECES;
constexpr auto STATUS_BAD_CASTLING_RIGHTS = Status::BAD_CASTLING_RIGHTS;
constexpr auto STATUS_INVALID_EP_SQUARE = Status::INVALID_EP_SQUARE;
constexpr auto STATUS_OPPOSITE_CHECK = Status::OPPOSITE_CHECK;
constexpr auto STATUS_EMPTY = Status::EMPTY;
constexpr auto STATUS_RACE_CHECK = Status::RACE_CHECK;
constexpr auto STATUS_RACE_OVER = Status::RACE_OVER;
constexpr auto STATUS_RACE_MATERIAL = Status::RACE_MATERIAL;
constexpr auto STATUS_TOO_MANY_CHECKERS = Status::TOO_MANY_CHECKERS;
constexpr auto STATUS_IMPOSSIBLE_CHECK = Status::IMPOSSIBLE_CHECK;

enum class Termination : int {
    // Enum with reasons for a game to be over.

    CHECKMATE,
    // See :func:`chess.Board.is_checkmate()`.
    STALEMATE,
    // See :func:`chess.Board.is_stalemate()`.
    INSUFFICIENT_MATERIAL,
    // See :func:`chess.Board.is_insufficient_material()`.
    SEVENTYFIVE_MOVES,
    // See :func:`chess.Board.is_seventyfive_moves()`.
    FIVEFOLD_REPETITION,
    // See :func:`chess.Board.is_fivefold_repetition()`.
    FIFTY_MOVES,
    // See :func:`chess.Board.can_claim_fifty_moves()`.
    THREEFOLD_REPETITION,
    // """See :func:`chess.Board.can_claim_threefold_repetition()`.
    VARIANT_WIN,
    // See :func:`chess.Board.is_variant_win()`.
    VARIANT_LOSS,
    // See :func:`chess.Board.is_variant_loss()`.
    VARIANT_DRAW,
    // See :func:`chess.Board.is_variant_draw()`.
};

struct Outcome {
    // Information about the outcome of an ended game, usually obtained from
    // :func:`chess.Board.outcome()`.

    Termination termination;
    std::optional<Color> winner;

    auto result() -> std::string {
        // """Returns ``1-0``, ``0-1`` or ``1/2-1/2``."""
        if (winner.has_value()) {
            return winner.value() ? "1-0" : "0-1";
        } else {
            return "1/2-1/2";
        }
    }
};

const std::array<std::string, 64> SQUARE_NAMES = {
    "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8",
    "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8",
    "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8",
    "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8",
    "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
    "G1", "G2", "G3", "G4", "G5", "G6", "G7", "G8",
    "H1", "H2", "H3", "H4", "H5", "H6", "H7", "H8",
};

auto parse_square(std::string name) -> Square {
    // """
    // Gets the square index for the given square *name*
    // (e.g., ``a1`` returns ``0``).

    // :raises: :exc:`ValueError` if the square name is invalid.
    // """
    return (Square) std::distance(
        SQUARE_NAMES.begin(),
        std::find(
            SQUARE_NAMES.begin(),
            SQUARE_NAMES.end(),
            name));
}

auto square_name(Square square) -> std::string {
    // """Gets the name of the square, like ``a3``."""
    return SQUARE_NAMES[(size_t)square];
}

using Bitboard = unsigned long long;

// most of the bitboard stuff is done in #include "BitboardNames.hpp";

const auto SAN_REGEX = std::regex("^([NBKRQ])?([a-h])?([1-8])?[\\-x]?([a-h][1-8])(=?[nbrqkNBRQK])?[\\+#]?\\Z");

const auto FEN_CASTLING_REGEX = std::regex("^(?:-|[KQABCautoGH]{0,2}[kqabcautogh]{0,2})\\Z");

struct Piece {
    
    // """A piece with type and color."""

    PieceType piece_type;
    // """The piece type."""

    Color color; 
    // """The piece color."""

    Piece(PieceType p, Color c) {
        piece_type = p;
        color = c;
    }

    auto symbol() -> std::string {
        // """
        // Gets the symbol ``P``, ``N``, ``B``, ``R``, ``Q`` or ``K`` for white
        // pieces or the lower-case variants for the black pieces.
        // """
        auto symbol = piece_symbol(piece_type);
        return color ? strtools::toupper(symbol) : symbol;
    }

    auto unicode_symbol(bool invert_color = false) -> std::string {
        // """
        // Gets the Unicode character for the piece.
        // """
        auto symbol = piece_symbol(piece_type);
        std::string symbol_cased;
        if (invert_color) {
            symbol_cased = color ? symbol : strtools::toupper(symbol);
        } else {
            symbol_cased = color ? strtools::toupper(symbol) : symbol;
        }
        return UNICODE_PIECE_SYMBOLS.at(symbol_cased);
    }

    auto __hash__() -> int {
        return (int)piece_type + (color ? -1 : 5);
    }

    auto __repr__() -> std::string {
        return "Piece.from_symbol("+symbol()+")";
    }
    auto __str__() -> std::string {
        return symbol();
    }
    auto _repr_svg_() -> std::string {
        // import chess.svg
        // return chess.svg.piece(*this, size=45);
        return "UNIMPLEMENTED";
    }

    static auto from_symbol(std::string symbol) -> Piece {
        // """
        // Creates a :class:`~chess.Piece` instance from a piece symbol.

        // :raises: :exc:`ValueError` if the symbol is invalid.
        // """
        auto key = strtools::tolower(symbol);
        PieceType ptype = (PieceType)std::distance(
            PIECE_SYMBOLS.begin(),
            std::find(
                PIECE_SYMBOLS.begin(),
                PIECE_SYMBOLS.end(),
                key));
        Color c = (Color)strtools::isupper(symbol);
        return Piece(ptype, c);
    }

    static auto from_symbol(char symbol) -> Piece {
        // """
        // Creates a :class:`~chess.Piece` instance from a piece symbol.

        // :raises: :exc:`ValueError` if the symbol is invalid.
        // """
        std::string str_symbol{1, symbol};

        auto key = strtools::tolower(str_symbol);
        PieceType ptype = (PieceType)std::distance(
            PIECE_SYMBOLS.begin(),
            std::find(
                PIECE_SYMBOLS.begin(),
                PIECE_SYMBOLS.end(),
                key));
        Color c = (Color)strtools::isupper(str_symbol);
        return Piece(ptype, c);
    }
};

struct Move {
    // """
    // Represents a move from a square to a square and possibly the promotion
    // piece type.

    // Drops and null moves are supported.
    // """

    Square from_square;
    // """The source square."""

    Square to_square;
    // """The target square."""

    std::optional<PieceType> promotion = std::nullopt;
    // """The promotion piece type or ``std::nullopt;``."""

    std::optional<PieceType> drop = std::nullopt;
    // """The drop piece type or ``std::nullopt;``."""

    Move(Square from_square, Square to_square, std::optional<PieceType> promotion = std::nullopt, std::optional<PieceType> drop = std::nullopt) {
        this->from_square = from_square;
        this->to_square = to_square;
        this->promotion = promotion;
        this->drop = drop;
    }

    auto uci() -> std::string {
        // """
        // Gets a UCI string for the move.

        // For example, a move from a7 to a8 would be ``a7a8`` or ``a7a8q``
        // (if the latter is a promotion to a queen).

        // The UCI representation of a null move is ``0000``.
        // """
        if (drop.has_value())
            return strtools::toupper(piece_symbol(drop.value())) + "@" + SQUARE_NAMES[to_square];
        else if (promotion.has_value())
            return SQUARE_NAMES[from_square] + SQUARE_NAMES[to_square] + piece_symbol(promotion.value());
        else if (__bool__())
            return SQUARE_NAMES[from_square] + SQUARE_NAMES[to_square];
        else
            return "0000";
    }

    auto xboard() -> std::string {
        return __bool__() ? uci() : "@@@@";
    }

    auto __bool__() -> bool {
        return (bool)(from_square | to_square | promotion.has_value() | drop.has_value());
    }

    auto __repr__() -> std::string {
        return "Move.from_uci(" +uci()+ ")";
    }

    auto __str__() -> std::string {
        return uci();
    }

    static auto from_uci(std::string uci) -> Move {
        // """
        // Parses a UCI string.

        // :raises: :exc:`ValueError` if the UCI string is invalid.
        // """
        if (uci == "0000") {
            return null();
        } else if (uci.size() == 4 && '@' == uci[1]) {
            auto drop = index(
                PIECE_SYMBOLS,
                std::tolower(uci[0]));
            auto square = index(
                SQUARE_NAMES,
                uci.substr(2, std::string::npos));
            return Move(
                (Square)square,
                (Square)square,
                std::nullopt,
                (PieceType)drop);
        } else if (4 <= uci.size() && uci.size() <= 5) {
            auto from_square = (Square)index(
                SQUARE_NAMES,
                uci.substr(0, 2));
            auto to_square = (Square)index(
                SQUARE_NAMES,
                uci.substr(2, 2));
            std::optional<Chess::PieceType> promotion;
            if (uci.size() == 5) {
                promotion = (PieceType)index(
                    PIECE_SYMBOLS,
                    uci[4]);
            } else {
                promotion = std::nullopt;
            }
            if (from_square == to_square) {
                auto msg = "invalid uci (use 0000 for null moves): "s + uci;
                throw std::invalid_argument(msg);
            }
            return Move(
                from_square,
                to_square,
                promotion,
                std::nullopt);
        } else {
            auto msg = "expected uci string to be of length 4 or 5: "s + uci;
            throw std::invalid_argument(msg);
        }
    }

    static auto null() -> Move {
        // """
        // Gets a null move.

        // A null move just passes the turn to the other side (and possibly
        // forfeits en passant capturing). Null moves evaluate to ``False`` in
        // boolean contexts.

        // >>> import chess
        // >>>
        // >>> bool(chess.Move.null())
        // False
        // """
        return Move(A1, A1);
    }
};

class BaseBoard {
   public:
    // """
    // A board representing the position of chess pieces. See
    // :class:`~chess.Board` for a full board with move generation.

    // The board is initialized with the standard chess starting position, unless
    // otherwise specified in the optional *board_fen* argument. If *board_fen*
    // is ``std::nullopt;``, an empty board is created.
    // """

    std::array<Bitboard, 2> occupied_co;
    Bitboard pawns;
    Bitboard knights;
    Bitboard bishops;
    Bitboard rooks;
    Bitboard queens;
    Bitboard kings;
    Bitboard promoted;
    Bitboard occupied;

    BaseBoard(std::optional<std::string> board_fen = STARTING_BOARD_FEN) {
        std::fill(
            occupied_co.begin(),
            occupied_co.end(),
            BB_EMPTY);

        if (!board_fen.has_value())
            _clear_board();
        else if (board_fen.value() == STARTING_BOARD_FEN)
            _reset_board();
        else
            _set_board_fen(board_fen.value());
    }

    void _reset_board() {
        pawns = BB_RANK_2 | BB_RANK_7;
        knights = BB_B1 | BB_G1 | BB_B8 | BB_G8;
        bishops = BB_C1 | BB_F1 | BB_C8 | BB_F8;
        rooks = BB_CORNERS;
        queens = BB_D1 | BB_D8;
        kings = BB_E1 | BB_E8;

        promoted = BB_EMPTY;

        occupied_co[WHITE] = BB_RANK_1 | BB_RANK_2;
        occupied_co[BLACK] = BB_RANK_7 | BB_RANK_8;
        occupied = BB_RANK_1 | BB_RANK_2 | BB_RANK_7 | BB_RANK_8;
    }

    void reset_board() {
        // """Resets pieces to the starting position."""
        _reset_board();
    }

    void _clear_board() {
        pawns = BB_EMPTY;
        knights = BB_EMPTY;
        bishops = BB_EMPTY;
        rooks = BB_EMPTY;
        queens = BB_EMPTY;
        kings = BB_EMPTY;

        promoted = BB_EMPTY;

        occupied_co[WHITE] = BB_EMPTY;
        occupied_co[BLACK] = BB_EMPTY;
        occupied = BB_EMPTY;
    }
    void clear_board() {
        // """Clears the board."""
        _clear_board();
    }

    auto pieces_mask(PieceType piece_type, Color color) -> Bitboard {
        Bitboard bb;
        if (piece_type == PieceType::PAWN) {
            bb = pawns;
        } else if (piece_type == PieceType::KNIGHT) {
            bb = knights;
        } else if (piece_type == PieceType::BISHOP) {
            bb = bishops;
        } else if (piece_type == PieceType::ROOK) {
            bb = rooks;
        } else if (piece_type == PieceType::QUEEN) {
            bb = queens;
        } else if (piece_type == PieceType::KING) {
            bb = kings;
        } else {
            std::cerr << "expected PieceType, got " << (int)piece_type;
            assert(false);
        }

        return bb & occupied_co[color];
    }

    auto pieces(PieceType piece_type, Color color) -> SquareSet {
        // """
        // Gets pieces of the given type and color.

        // Returns a :class:`set of squares <chess.SquareSet>`.
        // """
        return SquareSet(pieces_mask(piece_type, color));
    }

    auto piece_at(Square square) -> std::optional<Piece> {
        // """Gets the :class:`piece <chess.Piece>` at the given square."""
        auto piece_type = piece_type_at(square);
        if (piece_type.has_value()) {
            auto mask = BB_SQUARES[square];
            auto color = (Color)(occupied_co[WHITE] & mask);
            return Piece(piece_type.value(), color);
        } else {
            return std::nullopt;
        }
    }

    auto piece_type_at(Square square) -> std::optional<PieceType> {
        // """Gets the piece type at the given square."""
        auto mask = BB_SQUARES[square];

        if (!(occupied & mask))
            return std::nullopt;  // Early return
        else if (pawns & mask)
            return PieceType::PAWN;
        else if (knights & mask)
            return PieceType::KNIGHT;
        else if (bishops & mask)
            return PieceType::BISHOP;
        else if (rooks & mask)
            return PieceType::ROOK;
        else if (queens & mask)
            return PieceType::QUEEN;
        else
            return PieceType::KING;
    }

    auto color_at(Square square) -> std::optional<Color> {
        // """Gets the color of the piece at the given square."""
        auto mask = BB_SQUARES[square];
        if (occupied_co[WHITE] & mask)
            return WHITE;
        else if (occupied_co[BLACK] & mask)
            return BLACK;
        else
            return std::nullopt;
    }

    auto king(Color color) -> std::optional<Square> {
        // """
        // Finds the king square of the given side. Returns ``std::nullopt;`` if there
        // is no king of that color.

        // In variants with king promotions, only non-promoted kings are
        // considered.
        // """
        auto king_mask = occupied_co[color] & kings & ~promoted;
        if (king_mask) {
            return (Square)msb(king_mask);
        } else {
            return std::nullopt;
        } 
    }

    auto attacks_mask(Square square) -> Bitboard {
        auto bb_square = BB_SQUARES[square];

        if (bb_square & pawns) {
            auto color = (Color)(bb_square & occupied_co[WHITE]);
            return BB_PAWN_ATTACKS[color][square];
        } else if (bb_square & knights) {
            return BB_KNIGHT_ATTACKS[square];
        } else if (bb_square & kings) {
            return BB_KING_ATTACKS[square];
        } else {
            Bitboard attacks = 0;
            if (bb_square & bishops || bb_square & queens) {
                attacks = BB_DIAG_ATTACKS[square].at(BB_DIAG_MASKS[square] & occupied);
            }
            if (bb_square & rooks || bb_square & queens) {
                attacks |= (BB_RANK_ATTACKS[square].at(BB_RANK_MASKS[square] & occupied) | BB_FILE_ATTACKS[square].at(BB_FILE_MASKS[square] & occupied));
            }
            return attacks;
        }
    }

    auto attacks(Square square) -> SquareSet {
        // """
        // Gets the set of attacked squares from the given square.

        // There will be no attacks if the square is empty. Pinned pieces are
        // still attacking other squares.

        // Returns a :class:`set of squares <chess.SquareSet>`.
        // """
        return SquareSet(attacks_mask(square));
    }

    auto _attackers_mask(Color color, Square square, Bitboard occupied) -> Bitboard {
        auto rank_pieces = BB_RANK_MASKS[square] & occupied;
        auto file_pieces = BB_FILE_MASKS[square] & occupied;
        auto diag_pieces = BB_DIAG_MASKS[square] & occupied;

        auto queens_and_rooks = queens | rooks;
        auto queens_and_bishops = queens | bishops;

        auto attackers = ((BB_KING_ATTACKS[square] & kings) |
                              (BB_KNIGHT_ATTACKS[square] & knights) |
                              (BB_RANK_ATTACKS[square].at(rank_pieces) & queens_and_rooks) |
                              (BB_FILE_ATTACKS[square].at(file_pieces) & queens_and_rooks) |
                              (BB_DIAG_ATTACKS[square].at(diag_pieces) & queens_and_bishops) |
                              (BB_PAWN_ATTACKS[!color][square] & pawns));

        return attackers & occupied_co[color];
    }

    auto attackers_mask(Color color, Square square) -> Bitboard {
        return _attackers_mask(color, square, occupied);
    }

    auto is_attacked_by(Color color, Square square) -> bool {
        // """
        // Checks if the given side attacks the given square.

        // Pinned pieces still count as attackers. Pawns that can be captured
        // en passant are **not** considered attacked.
        // """
        return (bool)(attackers_mask(color, square));
    }

    auto attackers(Color color, Square square) -> SquareSet {
        // """
        // Gets the set of attackers of the given color for the given square.

        // Pinned pieces still count as attackers.

        // Returns a :class:`set of squares <chess.SquareSet>`.
        // """
        return SquareSet(attackers_mask(color, square));
    }

    auto pin_mask(Color color, Square square) -> Bitboard {
        auto king_square = king(color);
        if (!king_square.has_value())
            return BB_ALL;

        auto square_mask = BB_SQUARES[square];

        const std::array attacks_sliders = {
            std::make_pair(BB_FILE_ATTACKS, rooks | queens),
            std::make_pair(BB_RANK_ATTACKS, rooks | queens),
            std::make_pair(BB_DIAG_ATTACKS, bishops | queens),
        };
        for (auto [attacks_vecmap, sliders_bb] : attacks_sliders) {
            auto rays = attacks_vecmap[king_square.value()].at(0);
            if (rays & square_mask) {
                auto snipers = rays & sliders_bb & occupied_co[!color];
                for (auto sniper : scan_reversed(snipers)) {
                    if ((between(sniper, king_square.value()) & (occupied | square_mask)) == square_mask) {
                        return ray(king_square.value(), sniper);
                    }
                }

                break;
            }
        }
        return BB_ALL;
    }

    auto pin(Color color, Square square) -> SquareSet {
        // """
        // Detects an absolute pin (and its direction) of the given square to
        // the king of the given color.

        // >>> import chess
        // >>>
        // >>> board = chess.Board("rnb1k2r/ppp2ppp/5n2/3q4/1b1P4/2N5/PP3PPP/R1BQKBNR w KQkq - 3 7")
        // >>> board.is_pinned(chess.WHITE, chess.C3)
        // true
        // >>> direction = board.pin(chess.WHITE, chess.C3)
        // >>> direction
        // SquareSet(0x0000_0001_0204_0810)
        // >>> print(direction)
        // . . . . . . . .
        // . . . . . . . .
        // . . . . . . . .
        // 1 . . . . . . .
        // . 1 . . . . . .
        // . . 1 . . . . .
        // . . . 1 . . . .
        // . . . . 1 . . .

        // Returns a :class:`set of squares <chess.SquareSet>` that mask the rank,
        // file or diagonal of the pin. If there is no pin, then a mask of the
        // entire board is returned.
        // """
        return SquareSet(pin_mask(color, square));
    }

    auto is_pinned(Color color, Square square) -> bool {
        // """
        // Detects if the given square is pinned to the king of the given color.
        // """
        return pin_mask(color, square) != BB_ALL;
    }

    auto _remove_piece_at(Square square) -> std::optional<PieceType> {
        auto piece_type = piece_type_at(square);
        auto mask = BB_SQUARES[square];

        if (piece_type == PieceType::PAWN)
            pawns ^= mask;
        else if (piece_type == PieceType::KNIGHT)
            knights ^= mask;
        else if (piece_type == PieceType::BISHOP)
            bishops ^= mask;
        else if (piece_type == PieceType::ROOK)
            rooks ^= mask;
        else if (piece_type == PieceType::QUEEN)
            queens ^= mask;
        else if (piece_type == PieceType::KING)
            kings ^= mask;
        else
            return std::nullopt;

        occupied ^= mask;
        occupied_co[WHITE] &= ~mask;
        occupied_co[BLACK] &= ~mask;

        promoted &= ~mask;

        return piece_type;
    }

    auto remove_piece_at(Square square) -> std::optional<Piece> {
        // """
        // Removes the piece from the given square. Returns the
        // :class:`~chess.Piece` or ``std::nullopt;`` if the square was already empty.
        // """
        auto color = (Color)(occupied_co[WHITE] & BB_SQUARES[square]);
        auto piece_type = _remove_piece_at(square);
        if (piece_type.has_value()) {
            return Piece(piece_type.value(), color);
        } else {
            return std::nullopt;
        }
    }

    auto _set_piece_at(Square square, PieceType piece_type, Color color, bool was_promoted = false) {
        _remove_piece_at(square);

        auto mask = BB_SQUARES[square];

        if (piece_type == PieceType::PAWN)
            pawns |= mask;
        else if (piece_type == PieceType::KNIGHT)
            knights |= mask;
        else if (piece_type == PieceType::BISHOP)
            bishops |= mask;
        else if (piece_type == PieceType::ROOK)
            rooks |= mask;
        else if (piece_type == PieceType::QUEEN)
            queens |= mask;
        else if (piece_type == PieceType::KING)
            kings |= mask;
        else
            return;

        occupied ^= mask;
        occupied_co[color] ^= mask;

        if (was_promoted)
            promoted ^= mask;
    }

    auto set_piece_at(Square square, std::optional<Piece> piece, bool was_promoted = false) {
        // """
        // Sets a piece at the given square.

        // An existing piece is replaced. Setting *piece* to ``std::nullopt;`` is
        // equivalent to :func:`~chess.Board.remove_piece_at()`.
        // """
        if (!piece.has_value()) {
            _remove_piece_at(square);
        } else {
            auto p = piece.value();
            _set_piece_at(square, p.piece_type, p.color, was_promoted);
        }
    }

    auto board_fen(std::optional<bool> was_promoted = false) -> std::string {
        // """
        // Gets the board FEN (e.g.,
        // ``rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR``).
        // """
        std::vector<std::string> builder;
        auto empty = 0;

        for (auto isquare : SQUARES_180) {
            auto maybe_piece = piece_at((Square)isquare);

            if (!maybe_piece.has_value()) {
                empty += 1;
            } else {
                auto piece = maybe_piece.value();
                if (empty) {
                    builder.push_back(std::to_string(empty));
                    empty = 0;
                }
                builder.push_back(piece.symbol());
                if (was_promoted && BB_SQUARES[isquare] & promoted) {
                    builder.push_back("~");
                }
            }

            if (BB_SQUARES[isquare] & BB_FILE_H) {
                if (empty) {
                    builder.push_back(std::to_string(empty));
                    empty = 0;
                }

                if (isquare != H1) {
                    builder.push_back("/");
                }
            }
        }

        return strtools::join(builder, "");
    }

    void _set_board_fen(std::string fen) {
        // # Compatibility with set_fen().
        fen = strtools::strip(fen);
        if (strtools::contains(fen, " ")) {
            auto msg = "expected position part of fen, got multiple parts: "s + fen;
            throw std::invalid_argument(msg);
        }
        // # Ensure the FEN is valid.
        auto rows = strtools::split(fen, '/');
        if (rows.size() != 8) {
            auto msg = "expected 8 rows in position part of fen: "s + fen;
            throw std::invalid_argument(msg);
        }
        // # Validate each row.
        for (auto row : rows){
            auto field_sum = 0;
            auto previous_was_digit = false;
            auto previous_was_piece = false;

            for (auto c : row){
                if (strtools::contains("12345678", c)){
                    if (previous_was_digit) {
                        auto msg = "two subsequent digits in position part of fen: "s + fen;
                        throw std::invalid_argument(msg);
                    }
                    field_sum += strtools::to_int(c); // char parse to int
                    previous_was_digit = true;
                    previous_was_piece = false;
                } else if (c == '~') {
                    if (!previous_was_piece) {
                        auto msg = "'~' not after piece in position part of fen: "s + fen;
                        throw std::invalid_argument(msg);
                    }
                    previous_was_digit = false;
                    previous_was_piece = false;
                } else if (std::find(PIECE_SYMBOLS.begin(), PIECE_SYMBOLS.end(), strtools::tolower(c)) != PIECE_SYMBOLS.end()) {
                    field_sum += 1;
                    previous_was_digit = false;
                    previous_was_piece = true;
                } else {
                    auto msg = "invalid character in position part of fen: "s + fen;
                    throw std::invalid_argument(msg);
                }
            }
            if (field_sum != 8) {
                auto msg = "expected 8 columns per row in position part of fen: "s + fen;
                throw std::invalid_argument(msg);
            }
        }
        // Clear the board.
        _clear_board();

        // Put pieces on the board.
        auto square_index = 0;
        for (auto c : fen) {
            if (strtools::contains("12345678", c)) {
                square_index += strtools::to_int(c);
            } else if (std::find(PIECE_SYMBOLS.begin(), PIECE_SYMBOLS.end(), strtools::tolower(c)) != PIECE_SYMBOLS.end()) {
                auto piece = Piece::from_symbol(c);
                _set_piece_at((Square)SQUARES_180[square_index], piece.piece_type, piece.color);
                square_index += 1;
            } else if (c == '~') {
                promoted |= BB_SQUARES[SQUARES_180[square_index - 1]];
            }
        }
    }

    auto set_board_fen(std::string fen) {
        // """
        // Parses *fen* and sets up the board, where *fen* is the board part of
        // a FEN.

        // :raises: :exc:`ValueError` if syntactically invalid.
        // """
        _set_board_fen(fen);
    }

    auto piece_map(Bitboard mask = BB_ALL) {
        // """
        // Gets a dictionary of :class:`pieces <chess.Piece>` by square index.
        // """
        std::unordered_map<Square, Piece> result;
        for (auto square : scan_reversed(occupied & mask)) {
            assert(piece_at(square).has_value());
            result[square] = piece_at(square).value();
        }
        return result;
    }

    auto _set_piece_map(const std::unordered_map<Square, Piece>& pieces) {
        _clear_board();
        for (auto [square, piece] : pieces)
            _set_piece_at(square, piece.piece_type, piece.color);
    }

    auto set_piece_map(const std::unordered_map<Square, Piece>& pieces) {
        // """
        // Sets up the board from a dictionary of :class:`pieces <chess.Piece>`
        // by square index.
        // """
        _set_piece_map(pieces);
    }

    auto _set_chess960_pos(int scharnagl) {
        if (!(0 <= scharnagl && scharnagl <= 959)) {
            auto msg = "chess960 position index not 0 <= "s + std::to_string(scharnagl) + " <= 959"s;
            throw std::invalid_argument(msg);
        }

        auto divmod = [](int x, int y) {
            return std::make_pair(x / y, x % y);
        };
        // # See http://www.russellcottrell.com/Chess/Chess960.htm for
        // # a description of the algorithm.
        auto [n__, bw] = divmod(scharnagl, 4);
        auto [n_, bb] = divmod(n__, 4);
        auto [n, q] = divmod(n_, 6);

        int n1, n2;
        for (n1 = 0; n1 < 4; ++n1){
            n2 = n + (3 - n1) * (4 - n1); // 2 - 5
            if (n1 < n2 && 1 <= n2 && n2 <= 4)
                break;}

        // # Bishops.
        auto bw_file = bw * 2 + 1;
        auto bb_file = bb * 2;
        bishops = (BB_FILES[bw_file] | BB_FILES[bb_file]) & BB_BACKRANKS;

        // # Queens.
        auto q_file = q;
        q_file += (int)(std::min(bw_file, bb_file) <= q_file);
        q_file += (int)(std::max(bw_file, bb_file) <= q_file);
        queens = BB_FILES[q_file] & BB_BACKRANKS;

        auto used = std::vector{bw_file, bb_file, q_file};

        // # Knights.
        knights = BB_EMPTY;
        for (auto i = 0; i < 8; ++i) {
            if (std::find(used.begin(), used.end(), i) == used.end()) { // i not in used:
                if (n1 == 0 || n2 == 0) {
                    knights |= BB_FILES[i] & BB_BACKRANKS;
                    used.push_back(i);
                }
                n1 -= 1;
                n2 -= 1;
            }
        }
        // # RKR.
        for (auto i = 0; i < 8; ++i) {
            if (std::find(used.begin(), used.end(), i) == used.end()) {
                rooks = BB_FILES[i] & BB_BACKRANKS;
                used.push_back(i);
                break;
            }
        }
        for (auto i = 1; i < 8; ++i) {
            if (std::find(used.begin(), used.end(), i) == used.end()) {
                kings = BB_FILES[i] & BB_BACKRANKS;
                used.push_back(i);
                break;
            }
        }
        for (auto i = 2; i < 8; ++i) {
            if (std::find(used.begin(), used.end(), i) == used.end()) {
                rooks |= BB_FILES[i] & BB_BACKRANKS;
                break;
            }
        }

        // # Finalize.
        pawns = BB_RANK_2 | BB_RANK_7;
        occupied_co[WHITE] = BB_RANK_1 | BB_RANK_2;
        occupied_co[BLACK] = BB_RANK_7 | BB_RANK_8;
        occupied = BB_RANK_1 | BB_RANK_2 | BB_RANK_7 | BB_RANK_8;
        promoted = BB_EMPTY;
    }

    auto set_chess960_pos(int scharnagl) {
        // """
        // Sets up a Chess960 starting position given its index between 0 and 959.
        // Also see :func:`~chess.BaseBoard.from_chess960_pos()`.
        // """
        _set_chess960_pos(scharnagl);
    }

    auto chess960_pos() -> std::optional<int> {
        // """
        // Gets the Chess960 starting position index between 0 and 959,
        // or ``std::nullopt;``.
        // """
        if (occupied_co[WHITE] != BB_RANK_1 | BB_RANK_2)
            return std::nullopt;
        if (occupied_co[BLACK] != BB_RANK_7 | BB_RANK_8)
            return std::nullopt;
        if (pawns != BB_RANK_2 | BB_RANK_7)
            return std::nullopt;
        if (promoted)
            return std::nullopt;

        // # Piece counts.
        auto brnqk_vals = std::array{
            std::make_pair(bishops, 4),
            std::make_pair(rooks, 4),
            std::make_pair(knights, 4),
            std::make_pair(queens, 2),
            std::make_pair(kings, 2),
        };
        if (std::any_of(
                brnqk_vals.begin(),
                brnqk_vals.end(),
                [](auto pair) { 
                auto [pieces_bb, count] = pair;
                return popcount(pieces_bb) != count; })) {
            return std::nullopt;
        }
        // # Symmetry.
        if (std::any_of(
                brnqk_vals.begin(),
                brnqk_vals.end(),
                [](auto pair) {
                    auto [bb, _] = pair;
                    return ((BB_RANK_1 & bb) << 56) != (BB_RANK_8 & bb);
                })) {
            return std::nullopt;
        }
        // # Algorithm from ChessX, src/database/bitboard.cpp, r2254.
        auto x = bishops & (2 + 8 + 32 + 128);
        if (!x)
            return std::nullopt;
        auto bs1 = (lsb(x) - 1);  // 2
        auto cc_pos = bs1;
        x = bishops & (1 + 4 + 16 + 64);
        if (!x)
            return std::nullopt;
        auto bs2 = lsb(x) * 2;
        cc_pos += bs2;

        auto q = 0;
        auto qf = false;
        auto n0 = 0;
        auto n1 = 0;
        auto n0f = false;
        auto n1f = false;
        auto rf = 0;
        auto n0s = std::array{0, 4, 7, 9};
        
        for (auto square = A1; (int)square < (int)H1 + 1; square = (Square)((int)square + 1)) {
            auto bb = BB_SQUARES[square];
            if (bb & queens) {
                qf = true;
            } else if (bb & rooks || bb & kings) {
                if (bb & kings) {
                    if (rf != 1) {
                        return std::nullopt;
                    }
                } else {
                    rf += 1;
                }

                if (!qf) {
                    q += 1;
                }

                if (!n0f) {
                    n0 += 1;
                } else if (!n1f) {
                    n1 += 1;
                }
            } else if (bb & knights) {
                if (!qf) {
                    q += 1;
                }

                if (!n0f) {
                    n0f = true;
                } else if (!n1f) {
                    n1f = true;
                }
            }
        }
        if (n0 < 4 && n1f && qf) {
            cc_pos += q * 16;
            auto krn = n0s[n0] + n1;
            cc_pos += krn * 96;
            return cc_pos;
        } else {
            return std::nullopt;
        }
    }

    auto __repr__() -> std::string {
        auto name = "BaseBoard";
        auto fen = board_fen();
        return name + "("s + fen + ")"s;
    }

    auto __str__() -> std::string {
        std::vector<std::string> builder;

        for (auto square : SQUARES_180){
            auto piece = piece_at((Square)square);

            if (piece.has_value())
                builder.push_back(piece.value().symbol());
            else
                builder.push_back(".");

            if (BB_SQUARES[square] & BB_FILE_H){
                if (square != H1) {
                    builder.push_back("\n");}}
            else {
                builder.push_back(" ");}
        }
        return strtools::join(builder, "");
    }

    auto unicode(bool invert_color = false, bool borders = false, std::string empty_square = "⭘") -> std::string {
        // TODO
        return __str__();
        // """
        // Returns a string representation of the board with Unicode pieces.
        // Useful for pretty-printing to a terminal.

        // :param invert_color: Invert color of the Unicode pieces.
        // :param borders: Show borders and a coordinate margin.
        // """
        // builder = []
        // for rank_index in range(7, -1, -1):
        //     if borders:
        //         builder.append("  ")
        //         builder.append("-" * 17)
        //         builder.append("\n")

        //         builder.append(RANK_NAMES[rank_index])
        //         builder.append(" ")

        //     for file_index in range(8):
        //         square_index = square(file_index, rank_index)

        //         if borders:
        //             builder.append("|")
        //         elif file_index > 0:
        //             builder.append(" ")

        //         piece = piece_at(square_index)

        //         if piece:
        //             builder.append(piece.unicode_symbol(invert_color=invert_color))
        //         else:
        //             builder.append(empty_square)

        //     if borders:
        //         builder.append("|")

        //     if borders or rank_index > 0:
        //         builder.append("\n")

        // if borders:
        //     builder.append("  ")
        //     builder.append("-" * 17)
        //     builder.append("\n")
        //     builder.append("   a b c d e f g h")

        // return "".join(builder)
    }

    auto _repr_svg_() -> std::string {
        // import chess.svg
        // return chess.svg.board(board=*this, size=400)
        return "UNIMPLEMENTED";
    }

    friend bool operator==(BaseBoard a, BaseBoard b) {
        return (
            a.occupied == b.occupied &&
            a.occupied_co[WHITE] == b.occupied_co[WHITE] &&
            a.pawns == b.pawns &&
            a.knights == b.knights &&
            a.bishops == b.bishops &&
            a.rooks == b.rooks &&
            a.queens == b.queens &&
            a.kings == b.kings);
    }

    friend bool operator!=(BaseBoard a, BaseBoard b) {
        return !(a == b);
    }

    auto apply_transform(std::function<Bitboard(Bitboard)> f) {
        pawns = f(pawns);
        knights = f(knights);
        bishops = f(bishops);
        rooks = f(rooks);
        queens = f(queens);
        kings = f(kings);

        occupied_co[WHITE] = f(occupied_co[WHITE]);
        occupied_co[BLACK] = f(occupied_co[BLACK]);
        occupied = f(occupied);
        promoted = f(promoted);
    }

    auto transform(std::function<Bitboard(Bitboard)> f) -> BaseBoard {
        // """
        // Returns a transformed copy of the board by applying a bitboard
        // transformation function.

        // Available transformations include :func:`chess.flip_vertical()`,
        // :func:`chess.flip_horizontal()`, :func:`chess.flip_diagonal()`,
        // :func:`chess.flip_anti_diagonal()`, :func:`chess.shift_down()`,
        // :func:`chess.shift_up()`, :func:`chess.shift_left()`, and
        // :func:`chess.shift_right()`.

        // Alternatively, :func:`~chess.BaseBoard.apply_transform()` can be used
        // to apply the transformation on the board.
        // """
        auto board = *this;
        board.apply_transform(f);
        return board;
    }

    auto apply_mirror() {
        apply_transform(flip_vertical);
        std::swap(occupied_co[WHITE], occupied_co[BLACK]);
    }

    auto mirror() -> BaseBoard {
        // """
        // Returns a mirrored copy of the board.

        // The board is mirrored vertically and piece colors are swapped, so that
        // the position is equivalent modulo color.

        // Alternatively, :func:`~chess.BaseBoard.apply_mirror()` can be used
        // to mirror the board.
        // """
        auto board = *this;
        board.apply_mirror();
        return board;
    }

    auto copy() {
        return *this;
    }

    auto __copy__() {
        return *this;
    }

    auto __deepcopy__() {
        return *this;
    }

    static auto empty() {
        // """
        // Creates a new empty board. Also see
        // :func:`~chess.BaseBoard.clear_board()`.
        // """
        return BaseBoard(std::nullopt);
    }

    static auto from_chess960_pos(int scharnagl) {
        // """
        // Creates a new board, initialized with a Chess960 starting position.

        // >>> import chess
        // >>> import random
        // >>>
        // >>> board = chess.Board.from_chess960_pos(random.randint(0, 959))
        // """
        auto board = BaseBoard::empty();
        board.set_chess960_pos(scharnagl);
        return board;
    }
};

struct _BoardState {
    Bitboard occupied_w;
    Bitboard occupied_b;
    Bitboard pawns;
    Bitboard knights;
    Bitboard bishops;
    Bitboard rooks;
    Bitboard queens;
    Bitboard kings;
    Bitboard promoted;
    Bitboard occupied;
    Bitboard castling_rights;
    int halfmove_clock;
    int fullmove_number;
    std::optional<Square> ep_square;
    Color turn;

    _BoardState(const Board& board) {
        this->pawns = board.pawns;
        this->knights = board.knights;
        this->bishops = board.bishops;
        this->rooks = board.rooks;
        this->queens = board.queens;
        this->kings = board.kings;

        this->occupied_w = board.occupied_co[WHITE];
        this->occupied_b = board.occupied_co[BLACK];
        this->occupied = board.occupied;

        this->promoted = board.promoted;

        this->turn = board.turn;
        this->castling_rights = board.castling_rights;
        this->ep_square = board.ep_square;
        this->halfmove_clock = board.halfmove_clock;
        this->fullmove_number = board.fullmove_number;
    }

    auto restore(Board& board) {
        board.pawns = this->pawns;
        board.knights = this->knights;
        board.bishops = this->bishops;
        board.rooks = this->rooks;
        board.queens = this->queens;
        board.kings = this->kings;

        board.occupied_co[WHITE] = this->occupied_w;
        board.occupied_co[BLACK] = this->occupied_b;
        board.occupied = this->occupied;

        board.promoted = this->promoted;

        board.turn = this->turn;
        board.castling_rights = this->castling_rights;
        board.ep_square = this->ep_square;
        board.halfmove_clock = this->halfmove_clock;
        board.fullmove_number = this->fullmove_number;
    }
};

class Board : public BaseBoard {
   public:
    Bitboard castling_rights;
    int halfmove_clock;
    int fullmove_number;
    std::optional<Square> ep_square;
    Color turn;

    // """
    // A :class:`~chess.BaseBoard`, additional information representing
    // a chess position, and a :data:`move stack <chess.Board.move_stack>`.

    // Provides :data:`move generation <chess.Board.legal_moves>`, validation,
    // :func:`parsing <chess.Board.parse_san()>`, attack generation,
    // :func:`game end detection <chess.Board.is_game_over()>`,
    // and the capability to :func:`make <chess.Board.push()>` and
    // :func:`unmake <chess.Board.pop()>` moves.

    // The board is initialized to the standard chess starting position,
    // unless otherwise specified in the optional *fen* argument.
    // If *fen* is ``None``, an empty board is created.

    // Optionally supports *chess960*. In Chess960, castling moves are encoded
    // by a king move to the corresponding rook square.
    // Use :func:`chess.Board.from_chess960_pos()` to create a board with one
    // of the Chess960 starting positions.

    // It's safe to set :data:`~Board.turn`, :data:`~Board.castling_rights`,
    // :data:`~Board.ep_square`, :data:`~Board.halfmove_clock` and
    // :data:`~Board.fullmove_number` directly.

    // .. warning::
    //     It is possible to set up and work with invalid positions. In this
    //     case, :class:`~chess.Board` implements a kind of "pseudo-chess"
    //     (useful to gracefully handle errors or to implement chess variants).
    //     Use :func:`~chess.Board.is_valid()` to detect invalid positions.
    // """

    static inline const std::vector<std::string> aliases = {"Standard"s, "Chess"s, "Classical"s, "Normal"s, "Illegal"s, "From Position"s};
    static inline const std::optional<std::string> uci_variant = "chess";
    static inline const std::optional<std::string> xboard_variant = "normal";
    static inline const std::string starting_fen = STARTING_FEN;

    static inline const std::optional<std::string> tbw_suffix = ".rtbw"s;
    static inline const std::optional<std::string> tbz_suffix = ".rtbz"s;
    static inline const std::optional<const char*> tbw_magic = std::make_optional("\x71\xe8\x23\x5d");
    static inline const std::optional<const char*> tbz_magic = std::make_optional("\xd7\x66\x0c\xa5");
    static inline const std::optional<std::string> pawnless_tbw_suffix = std::nullopt;
    static inline const std::optional<std::string> pawnless_tbz_suffix = std::nullopt;
    static inline const std::optional<const char*> pawnless_tbw_magic = std::nullopt;
    static inline const std::optional<const char*> pawnless_tbz_magic = std::nullopt;
    static inline const bool connected_kings = false;
    static inline const bool one_king = true;
    static inline const bool captures_compulsory = false;

    // turn: Color
    // """The side to move (``chess.WHITE`` or ``chess.BLACK``)."""

    // castling_rights: Bitboard
    // """
    // Bitmask of the rooks with castling rights.

    // To test for specific squares:

    // >>> import chess
    // >>>
    // >>> board = chess.Board()
    // >>> bool(board.castling_rights & chess.BB_H1)  # White can castle with the h1 rook
    // True

    // To add a specific square:

    // >>> board.castling_rights |= chess.BB_A1

    // Use :func:`~chess.Board.set_castling_fen()` to set multiple castling
    // rights. Also see :func:`~chess.Board.has_castling_rights()`,
    // :func:`~chess.Board.has_kingside_castling_rights()`,
    // :func:`~chess.Board.has_queenside_castling_rights()`,
    // :func:`~chess.Board.has_chess960_castling_rights()`,
    // :func:`~chess.Board.clean_castling_rights()`.
    // """

    // ep_square: Optional[Square]
    // """
    // The potential en passant square on the third or sixth rank or ``None``.

    // Use :func:`~chess.Board.has_legal_en_passant()` to test if en passant
    // capturing would actually be possible on the next move.
    // """

    // fullmove_number: int
    // """
    // Counts move pairs. Starts at `1` and is incremented after every move
    // of the black side.
    // """

    // halfmove_clock: int
    // """The number of half-moves since the last capture or pawn move."""

    // promoted: Bitboard
    // """A bitmask of pieces that have been promoted."""

    bool chess960;
    // """
    // Whether the board is in Chess960 mode. In Chess960 castling moves are
    // represented as king moves to the corresponding rook square.
    // """

    std::vector<Move> move_stack; //: List[Move]
    // """
    // The move stack. Use :func:`Board.push() <chess.Board.push()>`,
    // :func:`Board.pop() <chess.Board.pop()>`,
    // :func:`Board.peek() <chess.Board.peek()>` and
    // :func:`Board.clear_stack() <chess.Board.clear_stack()>` for
    // manipulation.
    // """
    std::vector<_BoardState> _stack;

    Board(std::optional<std::string> fen = STARTING_FEN, bool chess960 = false) {
        BaseBoard(std::nullopt);

        chess960 = chess960;

        ep_square = std::nullopt;
        move_stack.clear();
        _stack.clear();

        if (!fen.has_value())
            clear();
        else if (fen == this->starting_fen)
            reset();
        else
            set_fen(fen);
    }

    auto legal_moves() {
        // """
        // A dynamic list of legal moves.

        // >>> import chess
        // >>>
        // >>> board = chess.Board()
        // >>> board.legal_moves.count()
        // 20
        // >>> bool(board.legal_moves)
        // True
        // >>> move = chess.Move.from_uci("g1f3")
        // >>> move in board.legal_moves
        // True

        // Wraps :func:`~chess.Board.generate_legal_moves()` and
        // :func:`~chess.Board.is_legal()`.
        // """
        return LegalMoveGenerator(*this);
    }
    
    auto pseudo_legal_moves() {
        // """
        // A dynamic list of pseudo-legal moves, much like the legal move list.

        // Pseudo-legal moves might leave or put the king in check, but are
        // otherwise valid. Null moves are not pseudo-legal. Castling moves are
        // only included if they are completely legal.

        // Wraps :func:`~chess.Board.generate_pseudo_legal_moves()` and
        // :func:`~chess.Board.is_pseudo_legal()`.
        // """
        return PseudoLegalMoveGenerator(*this);
    }

    void reset() {
        // """Restores the starting position."""
        turn = WHITE;
        castling_rights = BB_CORNERS;
        ep_square = std::nullopt;
        halfmove_clock = 0;
        fullmove_number = 1;

        reset_board();
    }

    void reset_board() {
        // """
        // Resets only pieces to the starting position. Use
        // :func:`~chess.Board.reset()` to fully restore the starting position
        // (including turn, castling rights, etc.).
        // """
        BaseBoard::reset_board();
        clear_stack();
    }

    void clear() {
        // """
        // Clears the board.

        // Resets move stack and move counters. The side to move is white. There
        // are no rooks or kings, so castling rights are removed.

        // In order to be in a valid :func:`~chess.Board.status()`, at least kings
        // need to be put on the board.
        // """
        turn = WHITE;
        castling_rights = BB_EMPTY;
        ep_square = std::nullopt;
        halfmove_clock = 0;
        fullmove_number = 1;

        clear_board();
    }
        
    void clear_board() {
        BaseBoard::clear_board();
        clear_stack();
    }

    void clear_stack() {
        // """Clears the move stack."""
        move_stack.clear();
        _stack.clear();
    }

    auto root() -> Board {
        // """Returns a copy of the root position."""
        if (_stack.size()) {
            auto board = Board(std::nullopt, chess960);
            _stack[0].restore(board);
            return board;
        } else {
            return copy(false);
        }
    }

    auto ply() -> int {
        // """
        // Returns the number of half-moves since the start of the game, as
        // indicated by :data:`~chess.Board.fullmove_number` and
        // :data:`~chess.Board.turn`.

        // If moves have been pushed from the beginning, this is usually equal to
        // ``len(board.move_stack)``. But note that a board can be set up with
        // arbitrary starting positions, and the stack can be cleared.
        // """
        return 2 * (fullmove_number - 1) + (turn == BLACK);
    }

    auto remove_piece_at(Square square) {
        auto piece = BaseBoard::remove_piece_at(square);
        clear_stack();
        return piece;
    }

    auto set_piece_at(Square square, std::optional<Chess::Piece> piece, bool promoted = false) {
        BaseBoard::set_piece_at(
            square, 
            piece, 
            promoted);
        clear_stack();
    }

    class EPIterator {
        using Bitboard = unsigned long long;
        using SqIt = sqgen::SquareIterator<int, sqgen::Reverse>;
        // STATE INFO
        const Board& board;
        SqIt capturer;

       public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Move;
        using pointer = value_type*;
        using reference = value_type&;

        EPIterator(const Board& board, Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) : board(board) {
            // guard for null generator
            if (!board.ep_square.has_value() || !BB_SQUARES[(size_t)board.ep_square.value()] & to_mask) {
                capturer = SqIt::sentinel();
                return;
            }
            if (BB_SQUARES[(size_t)board.ep_square.value()] & board.occupied) {
                capturer = SqIt::sentinel();
                return;
            }

            Bitboard capturers = (
                board.pawns & board.occupied_co[board.turn] & from_mask & 
                BB_PAWN_ATTACKS[!board.turn][board.ep_square.value()] & 
                BB_RANKS[board.turn ? 4 : 3]);
            this->capturer = SqIt(capturers);
        }
        EPIterator(const Board& board, bool is_sentinel) : board(board) {
            if (is_sentinel) {
                capturer = SqIt::sentinel();
                return;
            }
            // guard for null generator
            if (!board.ep_square.has_value() || !BB_SQUARES[(size_t)board.ep_square.value()] & BB_ALL) {
                capturer = SqIt::sentinel();
                return;
            }
            if (BB_SQUARES[(size_t)board.ep_square.value()] & board.occupied) {
                capturer = SqIt::sentinel();
                return;
            }

            Bitboard capturers = (board.pawns & board.occupied_co[board.turn] & BB_ALL &
                                  BB_PAWN_ATTACKS[!board.turn][board.ep_square.value()] &
                                  BB_RANKS[board.turn ? 4 : 3]);
            this->capturer = SqIt(capturers);
        }

        auto operator*() -> value_type {
            return Move(
                (Square)capturer.peek(), 
                board.ep_square.value());
        }

        // Prefix increment
        inline EPIterator& operator++() {
            ++capturer;
            // advance to next move
            return *this;
        }

        // Postfix increment
        inline EPIterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const EPIterator& a, const EPIterator& b) {
            return a.capturer == b.capturer;
        };

        friend bool operator!=(const EPIterator& a, const EPIterator& b) {
            return a.capturer != b.capturer;
        };

        static EPIterator sentinel(const Board& board) {
            return EPIterator(board, true);
        }

    };

    class CastleIterator {
        using Bitboard = unsigned long long;
        using SqIt = sqgen::SquareIterator<int, sqgen::Reverse>;
        // STATE INFO
        const Board& board;
        SqIt candidates;
        Bitboard bb_c, bb_d, bb_f, bb_g, king, rook, king_path, rook_path, king_to, rook_to;

       public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Move;
        using pointer = value_type*;
        using reference = value_type&;

        CastleIterator(const Board& board, Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) : board(board) {
            if (board.is_variant_end()) {
                candidates = SqIt::sentinel();
                return;
            }

            auto backrank = (board.turn == WHITE) ? BB_RANK_1 : BB_RANK_8;
            king = board.occupied_co[board.turn] & board.kings & ~board.promoted & backrank & from_mask;
            king = king & -king;
            if (!king)
                return;

            bb_c = BB_FILE_C & backrank;
            bb_d = BB_FILE_D & backrank;
            bb_f = BB_FILE_F & backrank;
            bb_g = BB_FILE_G & backrank;

            // SETUP OVER

            candidates = SqIt(
                board.clean_castling_rights() & backrank & to_mask);

            // LOOP RUN-UP #1

            auto cand1 = candidates.peek();

            rook = BB_SQUARES[cand1];

            auto a_side = rook < king;
            king_to = a_side ? bb_c : bb_g;
            rook_to = a_side ? bb_d : bb_f;

            king_path = between(
                (Square)msb(king), 
                (Square)msb(king_to));
            rook_path = between(
                (Square)cand1, 
                (Square)msb(rook_to));

            while (!is_yield_legal() && !stop_iteration()) {
                ++(*this);
            }
        }

        auto is_yield_legal() -> bool {
            return !((board.occupied ^ king ^ rook) & (king_path | rook_path | king_to | rook_to) || board._attacked_for_king(king_path | king, board.occupied ^ king) || board._attacked_for_king(king_to, board.occupied ^ king ^ rook ^ rook_to));
        }

        auto operator*() -> value_type {
            return board._from_chess960(board.chess960, msb(king), candidates.peek());
        }

        auto stop_iteration() -> bool {
            return candidates == SqIt::sentinel();
        }

        // Prefix increment
        inline CastleIterator& operator++() {
            ++candidates;
            auto candidate = candidates.peek();

            rook = BB_SQUARES[candidate];

            auto a_side = rook < king;
            king_to = a_side ? bb_c : bb_g;
            rook_to = a_side ? bb_d : bb_f;

            king_path = between(
                (Square)msb(king), 
                (Square)msb(king_to));
            rook_path = between(
                (Square)candidate, 
                (Square)msb(rook_to));

            // recurse till we have a safe yield
            while (!is_yield_legal() && !stop_iteration()) {
                ++(*this);
            }

            // advance to next move
            return *this;
        }

        // Postfix increment
        inline CastleIterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const CastleIterator& a, const CastleIterator& b) {
            return a.candidates == b.candidates;
        };

        friend bool operator!=(const CastleIterator& a, const CastleIterator& b) {
            return a.candidates != b.candidates;
        };

        static CastleIterator sentinel(const Board& board) {
            auto out = CastleIterator(board);
            out.candidates = SqIt::sentinel();
            return out;
        }
    };

    class PseudoLegalMoveIterator {
        using Bitboard = unsigned long long;
        using SquareGenerator = sqgen::SquareIterator<int, sqgen::Reverse>;
        enum GENERATION_STAGE {
            PIECE_MOVES,
            CASTLING_MOVES,
            PAWN_CAPTURE,
            PAWN_ADVANCE_SINGLE,
            PAWN_ADVANCE_DOUBLE,
            EN_PASSANT,
        };
        // STATE INFO
        const Board& board;
        Bitboard our_pieces;
        Bitboard non_pawns;
        SquareGenerator from_square;
        SquareGenerator to_square;
        CastleIterator castle_move;
        EPIterator ep_move;
        GENERATION_STAGE stage;

       public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Move;
        using pointer = value_type*;
        using reference = value_type&;

        PseudoLegalMoveIterator(const Board& board, Bitboard from_mask = BB_ALL, Bitboard to_mask = BB_ALL) : board(board), castle_move(board), ep_move(board) {
            our_pieces = board.occupied_co[board.turn];

            non_pawns = our_pieces & ~board.pawns & from_mask;
            from_square = SquareGenerator(non_pawns);
            
        }

        auto operator*() -> value_type {
            // return current move
        }

        // Prefix increment
        inline PseudoLegalMoveIterator& operator++() {
            // advance to next move
            return *this;
        }

        // Postfix increment
        inline PseudoLegalMoveIterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const PseudoLegalMoveIterator& a, const PseudoLegalMoveIterator& b) {
            return COMP_WITH_SENTINEL;
        };

        friend bool operator!=(const PseudoLegalMoveIterator& a, const PseudoLegalMoveIterator& b) {
            return COMP_WITH_SENTINEL;
        };

        static PseudoLegalMoveIterator sentinel() {
            return SENTINEL;
        }
    };

    class PseudoLegalMoveGenerator {
        const Board& board;
       public:
        PseudoLegalMoveGenerator(const Board& board) : board(board) {}
        auto begin() {
            return PseudoLegalMoveIterator(board);
        }
        auto end() {
            return PseudoLegalMoveIterator::sentinel();
        }
    };

    auto checkers_mask() -> Bitboard {
        auto kingpos = king(turn);
        return (kingpos.has_value()) ? attackers_mask((Color)!turn, kingpos.value()) : BB_EMPTY;
    }

    auto checkers() -> SquareSet {
        // """
        // Gets the pieces currently giving check.

        // Returns a :class:`set of squares <chess.SquareSet>`.
        // """
        return SquareSet(checkers_mask());
    }

    auto is_check() -> bool {
        // """Tests if the current side to move is in check."""
        return (bool)checkers_mask();
    }

    auto gives_check(Move move) -> bool {
        // """
        // Probes if the given move would put the opponent in check. The move
        // must be at least pseudo-legal.
        // """
        push(move);
        auto ischeck = is_check();
        pop();
        return ischeck;
    }

    auto is_into_check(Move move) -> bool {
        auto maybe_king_sq = king(turn);
        if (!maybe_king_sq.has_value())
            return false;

        auto king_sq = maybe_king_sq.value();
        // # If already in check, look if it is an evasion.
        auto checkers_bb = attackers_mask(not self.turn, king_sq);
        auto evasions = _generate_evasions(king_sq, checkers_bb, BB_SQUARES[move.from_square], BB_SQUARES[move.to_square]);
        if (checkers_bb && std::find(evasions.begin(), evasions.end(), move) != evasions.end())
            return true;

        return !_is_safe(king_sq, _slider_blockers(king_sq), move);
    }

    auto was_into_check() -> bool {
        auto king_sq = king((Color)!turn);
        return king_sq.has_value() && is_attacked_by(turn, king_sq.value());
    }

    auto is_pseudo_legal(Move move) -> bool {
        // # Null moves are not pseudo-legal.
        if (!move.__bool__())
            return false;

        // # Drops are not pseudo-legal.
        if (move.drop)
            return false;

        // # Source square must not be vacant.
        auto piece = piece_type_at(move.from_square);
        if (!piece.has_value())
            return false;

        // # Get square masks.
        auto from_mask = BB_SQUARES[move.from_square];
        auto to_mask = BB_SQUARES[move.to_square];

        // # Check turn.
        if (!occupied_co[turn] & from_mask)
            return false;

        // # Only pawns can promote and only on the backrank.
        if (move.promotion.has_value()){
            if (piece != PieceType::PAWN) {
                return false; }

            if (turn == WHITE && square_rank(move.to_square) != 7)
                return false;
            else if (turn == BLACK && square_rank(move.to_square) != 0)
                return false;
        }
        // # Handle castling.
        if (piece == PieceType::KING) {
            auto cmove = _from_chess960(chess960, move.from_square, move.to_square);

            auto cmoves = generate_castling_moves();
            if (std::find(cmoves.begin(), cmoves.end(), move) != cmoves.end())
                return true;
        }
        // # Destination square can not be occupied.
        if (occupied_co[turn] & to_mask)
            return false;

        // # Handle pawn moves.
        if (piece == PieceType::PAWN){
            auto pmoves = generate_pseudo_legal_moves(from_mask, to_mask);
            return std::find(pmoves.begin(), pmoves.end(), move) != pmoves.end();
        }
        // # Handle all other pieces.
        return (bool)(attacks_mask(move.from_square) & to_mask);
    }

    auto is_legal(Move move) -> bool {
        return !is_variant_end() && is_pseudo_legal(move) && !is_into_check(move);
    }

    auto is_variant_end() -> bool {
        // """
        // Checks if the game is over due to a special variant end condition.

        // Note, for example, that stalemate is not considered a variant-specific
        // end condition (this method will return ``False``), yet it can have a
        // special **result** in suicide chess (any of
        // :func:`~chess.Board.is_variant_loss()`,
        // :func:`~chess.Board.is_variant_win()`,
        // :func:`~chess.Board.is_variant_draw()` might return ``True``).
        // """
        return false;
        }

    auto is_variant_loss() -> bool {
        // """
        // Checks if the current side to move lost due to a variant-specific
        // condition.
        // """
        return false;
        }

    auto is_variant_win() -> bool {
        // """
        // Checks if the current side to move won due to a variant-specific
        // condition.
        // """
        return false;
        }

    auto is_variant_draw() -> bool {
        // """
        // Checks if a variant-specific drawing condition is fulfilled.
        // """
        return false;
        }

    auto is_game_over(bool claim_draw = false) -> bool {
        return outcome(claim_draw=claim_draw).has_value();
    }

    auto result(bool claim_draw = false) -> std::string {
        auto m_outcome = outcome(claim_draw=claim_draw);
        return m_outcome.has_value() ? m_outcome.value().result() : "*"s;
    }

    auto no_legal_moves() {
        auto lmoves = generate_legal_moves();
        return lmoves.begin() == lmoves.end();
    }

    auto outcome(bool claim_draw = false) -> std::optional<Outcome> {
        // """
        // Checks if the game is over due to
        // :func:`checkmate <chess.Board.is_checkmate()>`,
        // :func:`stalemate <chess.Board.is_stalemate()>`,
        // :func:`insufficient material <chess.Board.is_insufficient_material()>`,
        // the :func:`seventyfive-move rule <chess.Board.is_seventyfive_moves()>`,
        // :func:`fivefold repetition <chess.Board.is_fivefold_repetition()>`,
        // or a :func:`variant end condition <chess.Board.is_variant_end()>`.
        // Returns the :class:`chess.Outcome` if the game has ended, otherwise
        // ``None``.

        // Alternatively, use :func:`~chess.Board.is_game_over()` if you are not
        // interested in who won the game and why.

        // The game is not considered to be over by the
        // :func:`fifty-move rule <chess.Board.can_claim_fifty_moves()>` or
        // :func:`threefold repetition <chess.Board.can_claim_threefold_repetition()>`,
        // unless *claim_draw* is given. Note that checking the latter can be
        // slow.
        // """
        // # Variant support.
        if (is_variant_loss())
            return Outcome(Termination::VARIANT_LOSS, (Color)!turn);
        if (is_variant_win())
            return Outcome(Termination::VARIANT_WIN, turn);
        if (is_variant_draw())
            return Outcome(Termination::VARIANT_DRAW, std::nullopt);

        // # Normal game end.
        if (is_checkmate())
            return Outcome(Termination::CHECKMATE, (Color)!turn);
        if (is_insufficient_material())
            return Outcome(Termination::INSUFFICIENT_MATERIAL, std::nullopt);
        
        if (no_legal_moves())
            return Outcome(Termination::STALEMATE, std::nullopt);

        // # Automatic draws.
        if (is_seventyfive_moves())
            return Outcome(Termination::SEVENTYFIVE_MOVES, std::nullopt);
        if (is_fivefold_repetition())
            return Outcome(Termination::FIVEFOLD_REPETITION, std::nullopt);

        // # Claimable draws.
        if (claim_draw) {
            if (can_claim_fifty_moves())
                return Outcome(Termination::FIFTY_MOVES, std::nullopt);
            if (can_claim_threefold_repetition())
                return Outcome(Termination::THREEFOLD_REPETITION, std::nullopt);
}
        return std::nullopt;
    }

    auto is_checkmate() -> bool {
        // """Checks if the current position is a checkmate."""
        if (!is_check())
            return false;

        return no_legal_moves();
    }
    auto is_stalemate() -> bool {
        // """Checks if the current position is a stalemate."""
        if (is_check())
            return false;

        if (is_variant_end())
            return false;

        return no_legal_moves();
    }
    auto is_insufficient_material() -> bool {
        // """
        // Checks if neither side has sufficient winning material
        // (:func:`~chess.Board.has_insufficient_material()`).
        // """
        return has_insufficient_material(WHITE) && has_insufficient_material(BLACK);
    }
    auto has_insufficient_material(Color color) -> bool {
        // """
        // Checks if *color* has insufficient winning material.

        // This is guaranteed to return ``False`` if *color* can still win the
        // game.

        // The converse does not necessarily hold:
        // The implementation only looks at the material, including the colors
        // of bishops, but not considering piece positions. So fortress
        // positions or positions with forced lines may return ``False``, even
        // though there is no possible winning line.
        // """
        if (occupied_co[color] & (pawns | rooks | queens))
            return false;

        // # Knights are only insufficient material if:
        // # (1) We do not have any other pieces, including more than one knight.
        // # (2) The opponent does not have pawns, knights, bishops or rooks.
        // #     These would allow selfmate.
        if (occupied_co[color] & knights)
            return (popcount(occupied_co[color]) <= 2 &&
                    !(occupied_co[!color] & ~kings & ~queens));

        // # Bishops are only insufficient material if:
        // # (1) We do not have any other pieces, including bishops of the
        // #     opposite color.
        // # (2) The opponent does not have bishops of the opposite color,
        // #     pawns or knights. These would allow selfmate.
        if (occupied_co[color] & bishops) {
            auto same_color = !(bishops & BB_DARK_SQUARES) || !(bishops & BB_LIGHT_SQUARES);
            return same_color && !pawns && !knights;
        }
        return true;
    }
    auto _is_halfmoves(int n) -> bool {
        return halfmove_clock >= n && !no_legal_moves();
    }

    auto is_seventyfive_moves() -> bool {
        // """
        // Since the 1st of July 2014, a game is automatically drawn (without
        // a claim by one of the players) if the half-move clock since a capture
        // or pawn move is equal to or greater than 150. Other means to end a game
        // take precedence.
        // """
        return _is_halfmoves(150);
    }

    auto is_fivefold_repetition() -> bool {
        // """
        // Since the 1st of July 2014 a game is automatically drawn (without
        // a claim by one of the players) if a position occurs for the fifth time.
        // Originally this had to occur on consecutive alternating moves, but
        // this has since been revised.
        // """
        return is_repetition(5);
    }
    auto can_claim_draw() -> bool {
        // """
        // Checks if the player to move can claim a draw by the fifty-move rule or
        // by threefold repetition.

        // Note that checking the latter can be slow.
        // """
        return can_claim_fifty_moves() || can_claim_threefold_repetition();
    }
    auto is_fifty_moves() -> bool {
        return _is_halfmoves(100);
    }
    auto can_claim_fifty_moves() -> bool {
        // """
        // Checks if the player to move can claim a draw by the fifty-move rule.

        // Draw by the fifty-move rule can be claimed once the clock of halfmoves
        // since the last capture or pawn move becomes equal or greater to 100,
        // or if there is a legal move that achieves this. Other means of ending
        // the game take precedence.
        // """
        if (is_fifty_moves())
            return true;

        if (halfmove_clock >= 99) {
            for (auto move : generate_legal_moves()) {
                if (!is_zeroing(move)) {
                    push(move);
                    auto can_draw = is_fifty_moves();
                    pop();
                    if (can_draw) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    auto can_claim_threefold_repetition() -> bool {
        // """
        // Checks if the player to move can claim a draw by threefold repetition.

        // Draw by threefold repetition can be claimed if the position on the
        // board occured for the third time or if such a repetition is reached
        // with one of the possible legal moves.

        // Note that checking this can be slow: In the worst case
        // scenario, every legal move has to be tested and the entire game has to
        // be replayed because there is no incremental transposition table.
        // """
        auto transposition_key = _transposition_key();
        using HashType = decltype(transposition_key);
        auto transpositions = hscnt::HashCounter<HashType>();
        transpositions.add(transposition_key);

        // # Count positions.
        auto switchyard = std::vector<Move>();
        while (!move_stack.empty()) {
            auto move = pop();
            switchyard.push_back(move);

            if (is_irreversible(move))
                break;

            transpositions.add(_transposition_key());
        }
        while (!switchyard.empty()) {
            auto m = switchyard.back();
            push(m);
            switchyard.pop_back();
        }
        // # Threefold repetition occured.
        if (transpositions.count(transposition_key) >= 3)
            return true;

        // # The next legal move is a threefold repetition.
        auto lmoves = generate_legal_moves();
        for (auto move : lmoves) {
            self.push(move);
            auto key = _transposition_key();
            auto ret = transpositions.count(key) >= 2;
            self.pop();
            if (ret)
                return ret;
        }
        return false;
    }

    auto is_repetition(int count = 3) -> bool {
        // """
        // Checks if the current position has repeated 3 (or a given number of)
        // times.

        // Unlike :func:`~chess.Board.can_claim_threefold_repetition()`,
        // this does not consider a repetition that can be played on the next
        // move.

        // Note that checking this can be slow: In the worst case, the entire
        // game has to be replayed because there is no incremental transposition
        // table.
        // """
        // # Fast check, based on occupancy only.
        auto maybe_repetitions = 1;

        for (auto it = _stack.rbegin(); it != _stack.rend(); ++it) {
            auto state = *it;
            if (state.occupied == occupied) {
                maybe_repetitions += 1;
                if (maybe_repetitions >= count) {
                    break;
                }
            }
        }
        if (maybe_repetitions < count){
            return false;
        }
        // # Check full replay.
        auto transposition_key = _transposition_key();
        auto switchyard = std::vector<Move>();

        while (true) {
            if (count <= 1) {
                while (!switchyard.empty()) {
                    auto m = switchyard.back();
                    self.push(m);
                    switchyard.pop_back();
                }
                return true;
            }
            if (move_stack.size() < count - 1) {
                break;
            }

            auto move = pop();
            switchyard.push_back(move);

            if (is_irreversible(move)) {
                break;
            }
            if (_transposition_key() == transposition_key) {
                count--;
            }
        }
        while (!switchyard.empty()) {
            auto m = switchyard.back();
            self.push(m);
            switchyard.pop_back();
        }
        return false;
    }
};

}