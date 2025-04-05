#include "SDL3/sdl.h"
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <cstdlib> // for rand()
#include <ctime>   // for time()


using namespace std;

#define HEIGHT 800
#define WIDTH 800
#define SQUARE_SIZE 100

struct Color {
	int r, g, b, a;
};
struct SquarePos {
	float x, y;
};

struct CastlingRights {
	bool whiteKingside = true;
	bool whiteQueenside = true;
	bool blackKingside = true;
	bool blackQueenside = true;
};


const Color WHITE_COLOR = { 255, 255, 255, 255 };
const Color BLACK_COLOR = { 0, 0, 0, 255 };
const Color GRAY_COLOR = { 128, 128, 128, 255 };
const Color SUGGESTIONCOLOR = { 100, 100, 0, 255 };

class Piece {
public:
	static const int None = 0;
	static const int Pawn = 1;
	static const int Knight = 2;
	static const int Bishop = 3;
	static const int Rook = 4;
	static const int Queen = 5;
	static const int King = 6;

	static const int White = 0;
	static const int Black = 8;

	static const int WhitePawn = White | Pawn;
	static const int WhiteKnight = White | Knight;
	static const int WhiteBishop = White | Bishop;
	static const int WhiteRook = White | Rook;
	static const int WhiteQueen = White | Queen;
	static const int WhiteKing = White | King;

	static const int BlackPawn = Black | Pawn;
	static const int BlackKnight = Black | Knight;
	static const int BlackBishop = Black | Bishop;
	static const int BlackRook = Black | Rook;
	static const int BlackQueen = Black | Queen;
	static const int BlackKing = Black | King;

	static bool IsColor(int piece, int color) {
		return (piece & 8) == color;
	}

	static bool IsSlidingPiece(int piece) {
		int type = piece & 0b111;
		return (type == Bishop || type == Rook || type == Queen);
	}

	static bool MovesInDirection(int piece, int dir) {
		int type = piece & 0b111;
		if (type == Rook) return dir < 4;
		if (type == Bishop) return dir >= 4;
		if (type == Queen) return dir < 8;
		return false;
	}
	static int MakePiece(int color, int type) {
		return color | type;
	}
	static int GetOpponentColor(int color) {
		return color == White ? Black : White;
	}
	static int GetColor(int piece) {
		if (IsWhite(piece)) return White;
		if (IsBlack(piece)) return Black;
		return -1; // Invalid or empty square
	}
	static int oppositeColor(int color) {
		return (color == White) ? Black : White;
	}
	static bool IsKnight(int piece) {
		return piece == Piece::WhiteKnight || piece == Piece::BlackKnight;
	}
	static bool IsPawn(int piece) {
		return piece == Piece::WhitePawn || piece == Piece::BlackPawn;
	}
	static bool IsKing(int piece) {
		return piece == Piece::WhiteKing || piece == Piece::BlackKing;
	}
	static bool IsRook(int piece) {
		return piece == Piece::WhiteRook || piece == Piece::BlackRook;
	}
	static bool IsBishop(int piece) {
		return piece == Piece::WhiteBishop || piece == Piece::BlackBishop;
	}
	static bool IsQueen(int piece) {
		return piece == Piece::WhiteQueen || piece == Piece::BlackQueen;
	}
	static bool IsWhite(int piece) {
		return piece >= WhitePawn && piece <= WhiteKing;
	}
	static bool IsBlack(int piece) {
		return piece >= BlackPawn && piece <= BlackKing;
	}

	static int Type(int piece) {
		if (piece >= 1 && piece <= 6) return piece;       // White pieces
		else if (piece >= 9 && piece <= 14) return piece - 8; // Black pieces
		else return Piece::None;
	}
};

struct Move {
	int startSquare;
	int targetSquare;
	int capturedPiece = Piece::None; // store this so you can undo
	int movedPiece = Piece::None;
	enum Type {
		Normal,
		Capture,
		EnPassant,
		Promotion,
		KingsideCastle,
		QueensideCastle
	} type;

	// Optional: for promotion (to queen, rook, etc.)
	int promotionPiece = Piece::None;

	Move(int from, int to, Type t = Normal, int promo = Piece::None)
		: startSquare(from), targetSquare(to), type(t), promotionPiece(promo) {}
};

class PrecomputedMoveData {
public:
	static constexpr array<int, 8> DirectionOffsets = {
		8, -8, -1, 1, 7, -7, 9, -9
	};

	static array<array<int, 8>, 64> NumSquaresToEdge;
	static array<vector<int>, 64> rookMoves;
	static array<vector<int>, 64> bishopMoves;
	static array<vector<int>, 64> queenMoves;
	static array<vector<int>, 64> kingMoves;
	static array<vector<int>, 64> knightMoves;

	static void Init() {
		for (int file = 0; file < 8; ++file) {
			for (int rank = 0; rank < 8; ++rank) {
				int squareIndex = rank * 8 + file;

				int numNorth = 7 - rank;
				int numSouth = rank;
				int numWest = file;
				int numEast = 7 - file;

				NumSquaresToEdge[squareIndex][0] = numNorth;
				NumSquaresToEdge[squareIndex][1] = numSouth;
				NumSquaresToEdge[squareIndex][2] = numWest;
				NumSquaresToEdge[squareIndex][3] = numEast;
				NumSquaresToEdge[squareIndex][4] = std::min(numNorth, numWest);
				NumSquaresToEdge[squareIndex][5] = std::min(numSouth, numEast);
				NumSquaresToEdge[squareIndex][6] = std::min(numNorth, numEast);
				NumSquaresToEdge[squareIndex][7] = std::min(numSouth, numWest);

				// Sliding piece moves (rook, bishop, queen)
				for (int dir = 0; dir < 8; ++dir) {
					for (int n = 1; n <= NumSquaresToEdge[squareIndex][dir]; ++n) {
						int targetSquare = squareIndex + DirectionOffsets[dir] * n;

						if (dir <= 3) rookMoves[squareIndex].push_back(targetSquare);    // N, S, W, E
						else bishopMoves[squareIndex].push_back(targetSquare);          // diagonals

						queenMoves[squareIndex].push_back(targetSquare); // All 8 directions
					}
				}

				// King moves (1 square in each direction)
				for (int dir = 0; dir < 8; ++dir) {
					if (NumSquaresToEdge[squareIndex][dir] >= 1) {
						kingMoves[squareIndex].push_back(squareIndex + DirectionOffsets[dir]);
					}
				}

				// Knight moves (hardcoded offsets)
				std::vector<int> knightOffsets = { 17, 15, 10, 6, -17, -15, -10, -6 };
				for (int offset : knightOffsets) {
					int target = squareIndex + offset;

					// Check if within bounds (to prevent wraparound)
					int fileDiff = std::abs((squareIndex % 8) - (target % 8));
					int rankDiff = std::abs((squareIndex / 8) - (target / 8));

					if (target >= 0 && target < 64 && fileDiff <= 2 && rankDiff <= 2) {
						knightMoves[squareIndex].push_back(target);
					}
				}
			}
		}
	}
};

array<array<int, 8>, 64> PrecomputedMoveData::NumSquaresToEdge = {};
array<vector<int>, 64> PrecomputedMoveData::rookMoves = {};
array<vector<int>, 64> PrecomputedMoveData::bishopMoves = {};
array<vector<int>, 64> PrecomputedMoveData::queenMoves = {};
array<vector<int>, 64> PrecomputedMoveData::kingMoves = {};
array<vector<int>, 64> PrecomputedMoveData::knightMoves = {};

class Board {
public:
	int Square[64];
	int colorToMove = Piece::White;

	int enPassantSquare = -1; // -1 if not available
	bool whiteKingMoved = false;
	bool blackKingMoved = false;
	bool whiteKingsideRookMoved = false;
	bool whiteQueensideRookMoved = false;
	bool blackKingsideRookMoved = false;
	bool blackQueensideRookMoved = false;

	CastlingRights castlingRights;

	Board() {
		for (int i = 0; i < 64; i++) {
			Square[i] = Piece::None;
		}

		Square[0] = Piece::White | Piece::Rook;
		Square[1] = Piece::White | Piece::Knight;
		Square[2] = Piece::White | Piece::Bishop;
		Square[3] = Piece::White | Piece::Queen;
		Square[4] = Piece::White | Piece::King;
		Square[5] = Piece::White | Piece::Bishop;
		Square[6] = Piece::White | Piece::Knight;
		Square[7] = Piece::White | Piece::Rook;
		
		for (int i = 8; i < 16; i++) Square[i] = Piece::White | Piece::Pawn;
		for (int i = 48; i < 56; i++) Square[i] = Piece::Black | Piece::Pawn;
		
		Square[56] = Piece::Black | Piece::Rook;
		Square[57] = Piece::Black | Piece::Knight;
		Square[58] = Piece::Black | Piece::Bishop;
		Square[59] = Piece::Black | Piece::Queen;
		Square[60] = Piece::Black | Piece::King;
		Square[61] = Piece::Black | Piece::Bishop;
		Square[62] = Piece::Black | Piece::Knight;
		Square[63] = Piece::Black | Piece::Rook;
	}

	static bool IsPathClear(int from, int to, const Board& board) {
		int diff = to - from;

		// Determine direction
		for (int dir = 0; dir < 8; ++dir) {
			int offset = PrecomputedMoveData::DirectionOffsets[dir];
			for (int n = 1; n <= PrecomputedMoveData::NumSquaresToEdge[from][dir]; ++n) {
				int intermediate = from + offset * n;

				if (intermediate == to)
					return true; // path is clear, nothing in between

				if (board.Square[intermediate] != Piece::None)
					break; // something is blocking the way
			}
		}

		return false; // not a straight-line path or blocked
	}

	int findKingSquare(int color) const {
		for (int sq = 0; sq < 64; ++sq) {
			if (Piece::IsKing(Square[sq]) && Piece::IsColor(Square[sq], color))
				return sq;
		}
		return -1;
	}

	bool isSquareAttacked(int square, int byColor, const Board& board) {
		for (int from = 0; from < 64; ++from) {
			int piece = board.Square[from];
			if (!Piece::IsColor(piece, byColor))
				continue;

			int pieceType = Piece::Type(piece);

			switch (pieceType) {
			case Piece::Pawn: {
				int dir = (byColor == Piece::White) ? 1 : -1;
				int targetRank = from / 8 + dir;

				// Diagonal attacks only valid if targetRank is in bounds
				if (targetRank >= 0 && targetRank < 8) {
					int file = from % 8;
					if (file > 0 && from + dir * 8 - 1 == square) return true; // capture left
					if (file < 7 && from + dir * 8 + 1 == square) return true; // capture right
				}
				break;
			}

			case Piece::Knight: {
				for (int move : PrecomputedMoveData::knightMoves[from]) {
					if (move == square) return true;
				}
				break;
			}
			case Piece::Bishop: {
				for (int move : PrecomputedMoveData::bishopMoves[from]) {
					if (move == square) {
						if (Board::IsPathClear(from, move, board)) return true;
					}
				}
				break;
			}
			case Piece::Rook: {
				for (int move : PrecomputedMoveData::rookMoves[from]) {
					if (move == square) {
						if (Board::IsPathClear(from, move, board)) return true;
					}
				}
				break;
			}
			case Piece::Queen: {
				for (int move : PrecomputedMoveData::queenMoves[from]) {
					if (move == square) {
						if (Board::IsPathClear(from, move, board)) return true;
					}
				}
				break;
			}
			case Piece::King: {
				for (int move : PrecomputedMoveData::kingMoves[from]) {
					if (move == square) return true;
				}
				break;
			}
			}
		}

		return false; // No attackers found
	}

	void makeMove(const Move& m) {
		int movingPiece = Square[m.startSquare];
		int captured = Square[m.targetSquare];

		Square[m.targetSquare] = movingPiece;
		Square[m.startSquare] = Piece::None;

		// Save for undo
		const_cast<Move&>(m).movedPiece = movingPiece;
		const_cast<Move&>(m).capturedPiece = captured;

		// Handle castling
		if (m.type == Move::KingsideCastle) {
			int rookFrom = (colorToMove == Piece::White) ? 7 : 63;
			int rookTo = (colorToMove == Piece::White) ? 5 : 61;
			Square[rookTo] = Square[rookFrom];
			Square[rookFrom] = Piece::None;
		}
		else if (m.type == Move::QueensideCastle) {
			int rookFrom = (colorToMove == Piece::White) ? 0 : 56;
			int rookTo = (colorToMove == Piece::White) ? 3 : 59;
			Square[rookTo] = Square[rookFrom];
			Square[rookFrom] = Piece::None;
		}

		colorToMove = Piece::GetOpponentColor(colorToMove);
	}

	void undoMove(const Move& m) {
		Square[m.startSquare] = m.movedPiece;
		Square[m.targetSquare] = m.capturedPiece;

		// Undo castling
		if (m.type == Move::KingsideCastle) {
			int rookTo = (colorToMove == Piece::White) ? 5 : 61;
			int rookFrom = (colorToMove == Piece::White) ? 7 : 63;
			Square[rookFrom] = Square[rookTo];
			Square[rookTo] = Piece::None;
		}
		else if (m.type == Move::QueensideCastle) {
			int rookTo = (colorToMove == Piece::White) ? 3 : 59;
			int rookFrom = (colorToMove == Piece::White) ? 0 : 56;
			Square[rookFrom] = Square[rookTo];
			Square[rookTo] = Piece::None;
		}

		colorToMove = Piece::GetOpponentColor(colorToMove); // undo turn switch
	}


	char pieceChar(int piece) {
		switch (piece) {
		case Piece::WhitePawn: return 'P';
		case Piece::WhiteKnight: return 'N';
		case Piece::WhiteBishop: return 'B';
		case Piece::WhiteRook: return 'R';
		case Piece::WhiteQueen: return 'Q';
		case Piece::WhiteKing: return 'K';
		case Piece::BlackPawn: return 'p';
		case Piece::BlackKnight: return 'n';
		case Piece::BlackBishop: return 'b';
		case Piece::BlackRook: return 'r';
		case Piece::BlackQueen: return 'q';
		case Piece::BlackKing: return 'k';
		default: return '.';
		}
	}


	void printBoard(const Board& board) {
		for (int i = 0; i < 64; ++i) {
			std::cout << pieceChar(board.Square[i]) << " ";
			if (i % 8 == 7) std::cout << "\n";
		}
	}


	


};

class moveGenerator {

public:
	vector<Move> moves;

	vector<Move> GenerateMoves(Board* board) {
		moves.clear();

		for (int startSquare = 0; startSquare < 64; ++startSquare) {
			int piece = board->Square[startSquare];

			if (!Piece().IsColor(piece, board->colorToMove))
				continue;

			//cout << "Generating moves for square " << startSquare << ", piece: " << piece << endl;

			if (Piece().IsSlidingPiece(piece)) {
				GenerateSlidingMoves(startSquare, piece, board, moves);
			}
			else if (Piece().IsKnight(piece)) {
				GenerateKnightMoves(startSquare, piece, board, moves);
			}
			else if (Piece().IsPawn(piece)) {
				GeneratePawnMoves(startSquare, piece, board, moves);
			}
			else if (Piece().IsKing(piece)) {
				GenerateKingMoves(startSquare, piece, board, moves);
			}
		}
		return moves;
	}

	vector<Move> GenerateLegalMoves(Board* board) {
		vector<Move> allMoves = GenerateMoves(board);
		vector<Move> legal;

		for (const auto& move : allMoves) {
			Board copy = *board;
			copy.makeMove(move);

			int kingSq = copy.findKingSquare(board->colorToMove);
			if (!copy.isSquareAttacked(kingSq, Piece::GetOpponentColor(board->colorToMove), copy)) {
				legal.push_back(move);
			}
		}

		return legal;
	}

private:

	void GeneratePawnMoves(int startSquare, int piece, Board* board, vector<Move>& moves) {
		int direction = Piece::IsWhite(piece) ? 8 : -8;
		int startRank = Piece::IsWhite(piece) ? 1 : 6;
		int promotionRank = Piece::IsWhite(piece) ? 7 : 0;

		int forwardSquare = startSquare + direction;

		// 🟩 Forward Move
		if (forwardSquare >= 0 && forwardSquare < 64 && board->Square[forwardSquare] == Piece::None) {
			int rank = forwardSquare / 8;
			if (rank == promotionRank) {

				cout << "simple promotion \n";
				// 🔼 Promotion options
				moves.push_back(Move(startSquare, forwardSquare, Move::Promotion, Piece::Queen));
				moves.push_back(Move(startSquare, forwardSquare, Move::Promotion, Piece::Rook));
				moves.push_back(Move(startSquare, forwardSquare, Move::Promotion, Piece::Bishop));
				moves.push_back(Move(startSquare, forwardSquare, Move::Promotion, Piece::Knight));
			}
			else {
				moves.push_back({ startSquare, forwardSquare });
			}

			// 🟧 Double push
			int doublePush = startSquare + 2 * direction;
			if ((startSquare / 8) == startRank && board->Square[doublePush] == Piece::None) {
				moves.push_back({ startSquare, doublePush });
			}
		}

		// 🔻 En Passant
		int canEnPassant = board->enPassantSquare;
		if (canEnPassant != -1) {
			int leftCap = startSquare + direction - 1;
			int rightCap = startSquare + direction + 1;
			if (leftCap == canEnPassant && (startSquare % 8) != 0)
				moves.push_back({ startSquare, canEnPassant, Move::EnPassant });
			if (rightCap == canEnPassant && (startSquare % 8) != 7)
				moves.push_back({ startSquare, canEnPassant, Move::EnPassant });
		}

		// 🟥 Captures
		int captures[] = { direction - 1, direction + 1 };
		for (int capOffset : captures) {
			int targetSquare = startSquare + capOffset;
			if (targetSquare < 0 || targetSquare >= 64)
				continue;

			int startFile = startSquare % 8;
			int targetFile = targetSquare % 8;
			if (abs(startFile - targetFile) != 1)
				continue;

			int targetPiece = board->Square[targetSquare];
			if (targetPiece != Piece::None &&
				Piece::IsColor(targetPiece, Piece::GetOpponentColor(board->colorToMove))) {

				int rank = targetSquare / 8;
				if (rank == promotionRank) {
					cout << "not simmple promotion"<<endl;
					// 🔼 Promotion captures
					moves.push_back(Move(startSquare, targetSquare, Move::Promotion, Piece::Queen));
					moves.push_back(Move(startSquare, targetSquare, Move::Promotion, Piece::Rook));
					moves.push_back(Move(startSquare, targetSquare, Move::Promotion, Piece::Bishop));
					moves.push_back(Move(startSquare, targetSquare, Move::Promotion, Piece::Knight));
				}
				else {
					moves.push_back({ startSquare, targetSquare });
				}
			}
		}
	}



	void GenerateKnightMoves(int startSquare, int piece, Board* board, vector<Move>& moves) {
		static const int KnightOffsets[] = { -17, -15, -10, -6, 6, 10, 15, 17 };

		int rank = startSquare / 8;
		int file = startSquare % 8;

		for (int offset : KnightOffsets) {
			int targetSquare = startSquare + offset;
			if (targetSquare < 0 || targetSquare >= 64)
				continue;

			int newRank = targetSquare / 8;
			int newFile = targetSquare % 8;

			if (abs(rank - newRank) + abs(file - newFile) != 3)
				continue; // to prevent wrapping around the board

			int targetPiece = board->Square[targetSquare];

			if (targetPiece == Piece::None || Piece::IsColor(targetPiece, Piece::GetOpponentColor(board->colorToMove))) {
				moves.push_back({ startSquare, targetSquare });
			}
		}
	}


	void GenerateSlidingMoves(int startSquare, int piece, Board* board, vector<Move>& moves) {
		vector<int> directions;

		if (Piece().IsRook(piece) || Piece().IsQueen(piece)) {
			int rookDirs[] = { 8, -8, 1, -1 };
			directions.insert(directions.end(), begin(rookDirs), end(rookDirs));
		}
		if (Piece().IsBishop(piece) || Piece().IsQueen(piece)) {
			int bishopDirs[] = { 9, -9, 7, -7 };
			directions.insert(directions.end(), begin(bishopDirs), end(bishopDirs));
		}

		for (int dir : directions) {
			int targetSquare = startSquare;

			while (true) {
				int rank = targetSquare / 8;
				int file = targetSquare % 8;

				targetSquare += dir;

				if (targetSquare < 0 || targetSquare >= 64)
					break;

				int newRank = targetSquare / 8;
				int newFile = targetSquare % 8;

				if (abs(newFile - file) > 1 && (dir == 1 || dir == -1 || dir == 9 || dir == -9 || dir == 7 || dir == -7))
					break;

				int targetPiece = board->Square[targetSquare];

				if (targetPiece == Piece::None) {
					moves.push_back({ startSquare, targetSquare });
				}
				else {
					if (Piece::IsColor(targetPiece, Piece::GetOpponentColor(board->colorToMove))) {
						moves.push_back({ startSquare, targetSquare });
					}
					break; // blocked
				}
			}
		}
	}


	void GenerateKingMoves(int startSquare, int piece, Board* board, vector<Move>& moves) {
		static const int KingOffsets[] = {
			-9, -8, -7,
			-1,      1,
			 7,  8,  9
		};

		int rank = startSquare / 8;
		int file = startSquare % 8;

		for (int offset : KingOffsets) {
			int targetSquare = startSquare + offset;

			if (targetSquare < 0 || targetSquare >= 64)
				continue;

			int targetRank = targetSquare / 8;
			int targetFile = targetSquare % 8;

			if (abs(targetFile - file) > 1 || abs(targetRank - rank) > 1)
				continue;

			int targetPiece = board->Square[targetSquare];

			if (targetPiece == Piece::None ||
				Piece::IsColor(targetPiece, Piece::GetOpponentColor(board->colorToMove))) {
				moves.push_back(Move(startSquare, targetSquare));
			}
		}

		// Castling part
		int color = Piece::GetColor(piece);
		bool kingMoved = (color == Piece::White) ? board->whiteKingMoved : board->blackKingMoved;
		bool kingsideRookMoved = (color == Piece::White) ? board->whiteKingsideRookMoved : board->blackKingsideRookMoved;
		bool queensideRookMoved = (color == Piece::White) ? board->whiteQueensideRookMoved : board->blackQueensideRookMoved;

		int homeRank = (color == Piece::White) ? 0 : 7;
		int kingStart = homeRank * 8 + 4;
		int kingsideRook = homeRank * 8 + 7;
		int queensideRook = homeRank * 8 + 0;

		// Kingside
		if (!kingMoved && !kingsideRookMoved &&
			board->Square[kingStart + 1] == Piece::None &&
			board->Square[kingStart + 2] == Piece::None &&
			!board->isSquareAttacked(kingStart, Piece::GetOpponentColor(color), *board) &&
			!board->isSquareAttacked(kingStart + 1, Piece::GetOpponentColor(color), *board) &&
			!board->isSquareAttacked(kingStart + 2, Piece::GetOpponentColor(color), *board)) {

			moves.push_back(Move(kingStart, kingStart + 2, Move::KingsideCastle));
		}

		// Queenside
		if (!kingMoved && !queensideRookMoved &&
			board->Square[kingStart - 1] == Piece::None &&
			board->Square[kingStart - 2] == Piece::None &&
			board->Square[kingStart - 3] == Piece::None &&
			!board->isSquareAttacked(kingStart, Piece::GetOpponentColor(color), *board) &&
			!board->isSquareAttacked(kingStart - 1, Piece::GetOpponentColor(color), *board) &&
			!board->isSquareAttacked(kingStart - 2, Piece::GetOpponentColor(color), *board)) {

			moves.push_back(Move(kingStart, kingStart - 2, Move::QueensideCastle));
		}
	}

};

class Renderer {
public:
	static void setColor(SDL_Renderer* renderer, Color color) {
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	}

	static void drawSquare(SDL_Renderer* renderer, SquarePos pos, Color color) {
		SDL_FRect rect = { pos.x, pos.y, SQUARE_SIZE, SQUARE_SIZE };
		setColor(renderer, color);
		SDL_RenderFillRect(renderer, &rect);
	}

	static void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, Color color) {
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
		for (int w = 0; w < radius * 2; w++) {
			for (int h = 0; h < radius * 2; h++) {
				int dx = radius - w;
				int dy = radius - h;
				if ((dx * dx + dy * dy) <= (radius * radius)) {
					SDL_RenderPoint(renderer, centerX + dx, centerY + dy);
				}
			}
		}
	}

	
};

class BoardUi {
private:
	SDL_Texture* textures[12] = { nullptr };
	Board* logicBoard;  // Pointer to logic class

public:
	BoardUi(Board* b) {
		logicBoard = b;
	}

	void draw(SDL_Renderer* renderer, int selectedSquare = -1, const vector<Move>& legalMoves = {}) {
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				bool isLightSquare = (file + rank) % 2 != 0;
				Color squareColor = isLightSquare ? WHITE_COLOR : GRAY_COLOR;
				SquarePos pos = { file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE };
				Renderer::drawSquare(renderer, pos, squareColor);
			}
		}	
	}

	void selectedSquareHighLight(SDL_Renderer* renderer, int selectedSquare = -1) {
		if (selectedSquare != -1) {
			SquarePos pos = { (selectedSquare % 8) * SQUARE_SIZE, (7 - selectedSquare / 8) * SQUARE_SIZE };
			Renderer::drawSquare(renderer, pos, { 0, 255, 0, 100 }); // green overlay
		}
	}

	void highLightLegalMoves(SDL_Renderer* renderer, int selectedSquare = -1, const vector<Move>& legalMoves = {}) {
		for (const Move& move : legalMoves) {
			if (move.startSquare == selectedSquare) {
				int file = move.targetSquare % 8;
				int rank = 7 - (move.targetSquare / 8);
				int centerX = file * SQUARE_SIZE + SQUARE_SIZE / 2;
				int centerY = rank * SQUARE_SIZE + SQUARE_SIZE / 2;
				Renderer::drawCircle(renderer, centerX, centerY, SQUARE_SIZE / 6, SUGGESTIONCOLOR);
			}
		}
	}

	void loadTextures(SDL_Renderer* renderer) {
		string pieces[12] = {
			"WP.bmp", "WN.bmp", "WB.bmp", "WR.bmp", "WQ.bmp", "WK.bmp",
			"BP.bmp", "BN.bmp", "BB.bmp", "BR.bmp", "BQ.bmp", "BK.bmp"
		};

		for (int i = 0; i < 12; i++) {
			string filePath = "assets/" + pieces[i];
			SDL_Surface* surface = SDL_LoadBMP(filePath.c_str());
			if (!surface) {
				printf("Failed to load %s: %s\n", pieces[i].c_str(), SDL_GetError());
				continue;
			}
			textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_DestroySurface(surface);
		}
	}

	SDL_Texture* loadPieceTexture(SDL_Renderer* renderer, const std::string& filePath) {
		string filePath1 = "assets/" + filePath;
		cout << filePath1<<endl;
		SDL_Surface* surface = SDL_LoadBMP(filePath1.c_str());
		if (!surface) {
			std::cerr << "Failed to load image: " << filePath1 << std::endl;
			return nullptr;
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_DestroySurface(surface);

		if (!texture) {
			std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
		}

		return texture;
	}

	void renderPieces(SDL_Renderer* renderer) {
		for (int i = 0; i < 64; i++) {
			int piece = logicBoard->Square[i];
			if (piece == Piece::None) continue;

			int color = (piece & Piece::Black) ? 1 : 0;
			int type = piece & 7;

			int textureIndex = color * 6 + (type - 1);
			if (textureIndex < 0 || textureIndex >= 12) {
				printf("Invalid texture index %d for piece code %d\n", textureIndex, piece);
				continue;
			}

			int file = i % 8;
			int rank = i / 8;
			SDL_FRect rect = { file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE };
			SDL_RenderTexture(renderer, textures[textureIndex], NULL, &rect);
		}
	}

	int showPromotionWindow(SDL_Renderer* renderer, int color) {
		SDL_Window* promoWin = SDL_CreateWindow("Choose Promotion Piece",
			400, 100, 0);

		SDL_Renderer* promoRenderer = SDL_CreateRenderer(promoWin, NULL);

		// Load your piece textures here (replace with actual image loading)
		SDL_Texture* queenTex = loadPieceTexture(promoRenderer, color == Piece::White ? "WQ.bmp" : "BQ.bmp");
		SDL_Texture* rookTex = loadPieceTexture(promoRenderer, color == Piece::White ? "WR.bmp" : "BR.bmp");
		SDL_Texture* bishopTex = loadPieceTexture(promoRenderer, color == Piece::White ? "WB.bmp" : "BB.bmp");
		SDL_Texture* knightTex = loadPieceTexture(promoRenderer, color == Piece::White ? "WN.bmp" : "BN.bmp");

		SDL_FRect buttons[4];
		for (int i = 0; i < 4; i++) {
			buttons[i] = { static_cast<float>(i * 80), 0.0f, 100.0f, 100.0f };
		}

		SDL_Event e;
		bool running = true;
		int selected = -1;

		while (running) {
			SDL_RenderClear(promoRenderer);
			SDL_RenderTexture(promoRenderer, queenTex, NULL, &buttons[0]);
			SDL_RenderTexture(promoRenderer, rookTex, NULL, &buttons[1]);
			SDL_RenderTexture(promoRenderer, bishopTex, NULL, &buttons[2]);
			SDL_RenderTexture(promoRenderer, knightTex, NULL, &buttons[3]);
			SDL_RenderPresent(promoRenderer);

			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_EVENT_QUIT) {
					running = false;
				}
				else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
					int x = e.button.x;
					for (int i = 0; i < 4; i++) {
						if (x >= buttons[i].x && x < buttons[i].x + 100) {
							selected = i;
							running = false;
							break;
						}
					}
				}
			}
		}

		SDL_DestroyTexture(queenTex);
		SDL_DestroyTexture(rookTex);
		SDL_DestroyTexture(bishopTex);
		SDL_DestroyTexture(knightTex);
		SDL_DestroyRenderer(promoRenderer);
		SDL_DestroyWindow(promoWin);

		// Return piece type
		switch (selected) {
		case 0: return Piece::Queen;
		case 1: return Piece::Rook;
		case 2: return Piece::Bishop;
		case 3: return Piece::Knight;
		default: return Piece::Queen; // fallback
		}
	}
};

class Game {
private:
	Board board;
	BoardUi boardUi;
	moveGenerator generator;
	int selectedSquare = -1;
	vector<Move> legalMoves;
	bool gameOver = false;
	string resultMessage = "";
	int getSquareFromMouse(int x, int y) {
		int file = x / SQUARE_SIZE;
		int rank = 7 - (y / SQUARE_SIZE); // rank 0 at bottom
		return rank * 8 + file;
	}

public:
	Game(SDL_Renderer* renderer) : boardUi(&board) {
		boardUi.loadTextures(renderer);
		PrecomputedMoveData::Init();
	}

	bool isOver() const {
		return gameOver;
	}

	void handleEvent(SDL_Event& e, SDL_Renderer* renderer) {
	if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
		int mouseX = e.button.x;
		int mouseY = e.button.y;
		int clickedSquare = getSquareFromMouse(mouseX, mouseY);
		int clickedPiece = board.Square[clickedSquare];

		if (selectedSquare == -1) {
			if (Piece::IsColor(clickedPiece, board.colorToMove)) {
				selectedSquare = clickedSquare;
				legalMoves = generator.GenerateLegalMoves(&board);
				cout << "Selected square: " << selectedSquare << "\nLegal moves:\n";
				for (const auto& m : legalMoves)
					if (m.startSquare == selectedSquare)
						cout << m.startSquare << " -> " << m.targetSquare << endl;
			}
		}
		else {
			bool moved = false;
			for (const Move& m : legalMoves) {
				if (m.startSquare == selectedSquare && m.targetSquare == clickedSquare) {
					int movedPiece = board.Square[m.startSquare];
					int diff = m.targetSquare - m.startSquare;

					// En Passant capture
					if (m.type == Move::EnPassant) {
						int capturedPawnSq = (board.colorToMove == Piece::White) ? m.targetSquare - 8 : m.targetSquare + 8;
						board.Square[capturedPawnSq] = Piece::None;
					}

					// Move the piece
					board.Square[m.targetSquare] = movedPiece;
					board.Square[m.startSquare] = Piece::None;

					// Handle pawn promotion
					if (Piece::Type(movedPiece) == Piece::Pawn &&
						(m.targetSquare / 8 == 0 || m.targetSquare / 8 == 7)) {
							
						int promotionType = boardUi.showPromotionWindow(renderer, Piece::GetColor(movedPiece));
						board.Square[m.targetSquare] = Piece::MakePiece(Piece::GetColor(movedPiece), promotionType);
					}
					else {
						board.Square[m.targetSquare] = movedPiece;
					}


					// Handle castling
					if (m.type == Move::KingsideCastle) {
						int rookFrom = (board.colorToMove == Piece::White) ? 7 : 63;
						int rookTo = (board.colorToMove == Piece::White) ? 5 : 61;
						board.Square[rookTo] = board.Square[rookFrom];
						board.Square[rookFrom] = Piece::None;
					}
					else if (m.type == Move::QueensideCastle) {
						int rookFrom = (board.colorToMove == Piece::White) ? 0 : 56;
						int rookTo = (board.colorToMove == Piece::White) ? 3 : 59;
						board.Square[rookTo] = board.Square[rookFrom];
						board.Square[rookFrom] = Piece::None;
					}

					// Update en passant target
					if (Piece::Type(movedPiece) == Piece::Pawn && abs(diff) == 16) {
						board.enPassantSquare = (m.startSquare + m.targetSquare) / 2;
					}
					else {
						board.enPassantSquare = -1;
					}

					// Update castling rights if king or rook moved
					if (Piece::Type(movedPiece) == Piece::King) {
						if (board.colorToMove == Piece::White) {
							board.whiteKingMoved = true;
							board.castlingRights.whiteKingside = false;
							board.castlingRights.whiteQueenside = false;
						}
						else {
							board.blackKingMoved = true;
							board.castlingRights.blackKingside = false;
							board.castlingRights.blackQueenside = false;
						}
					}
					else if (Piece::Type(movedPiece) == Piece::Rook) {
						if (board.colorToMove == Piece::White) {
							if (m.startSquare == 0) {
								board.whiteQueensideRookMoved = true;
								board.castlingRights.whiteQueenside = false;
							}
							if (m.startSquare == 7) {
								board.whiteKingsideRookMoved = true;
								board.castlingRights.whiteKingside = false;
							}
						}
						else {
							if (m.startSquare == 56) {
								board.blackQueensideRookMoved = true;
								board.castlingRights.blackQueenside = false;
							}
							if (m.startSquare == 63) {
								board.blackKingsideRookMoved = true;
								board.castlingRights.blackKingside = false;
							}
						}
					}

					// Switch turn
					board.colorToMove = Piece::GetOpponentColor(board.colorToMove);

					moved = true;
					break;
				}
			}

			selectedSquare = -1;
			legalMoves.clear();

			isGameOver(board);
			}
		}
	}

	void isGameOver(Board& board) {
		// After bot move, update legal moves for player
		legalMoves = generator.GenerateLegalMoves(&board);
		
		// Check for check, checkmate or stalemate
		int kingSq = board.findKingSquare(board.colorToMove);
		bool inCheck = board.isSquareAttacked(kingSq, Piece::oppositeColor(board.colorToMove), board);
		vector<Move> nextMoves = generator.GenerateLegalMoves(&board);
		
		if (nextMoves.empty()) {
			if (inCheck)
			{
				gameOver = true;
				cout << "Checkmate! " << ((board.colorToMove == Piece::White) ? "Black" : "White") << " wins!\n";
			}
			else
			{
				gameOver = true;
				cout << "Stalemate! It's a draw!\n";
			}
		}
		else if (inCheck) {
			cout << "Check!\n";
		}
	}
	
	void update(SDL_Renderer* renderer) {
		// --- Drawing code ---
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
	
		boardUi.draw(renderer, selectedSquare, legalMoves);
		boardUi.selectedSquareHighLight(renderer, selectedSquare);
		boardUi.renderPieces(renderer);
		boardUi.highLightLegalMoves(renderer, selectedSquare, legalMoves);
	
		SDL_RenderPresent(renderer);
	}
};

int main() {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("Chess", WIDTH, HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

	Game game(renderer);
	srand(static_cast<unsigned>(time(0)));

	bool quit = false;
	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT)
				quit = true;

			if (!game.isOver()) {
				game.handleEvent(e, renderer);
			}
		}

		if (!game.isOver()) {
			game.update(renderer);
		}

		game.update(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
