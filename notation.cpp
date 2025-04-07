﻿#include "notation.h"

using namespace std;

string notation::indexToSquare(int index) {
	char file = 'a' + (index % 8);
	char rank = '1' + (7 - index / 8); // flip rank so 0 = a8
	return string() + file + rank;
}

char notation::pieceToChar(int piece) {
	switch (piece) {
	case Piece::Knight: return 'N';
	case Piece::Bishop: return 'B';
	case Piece::Rook:   return 'R';
	case Piece::Queen:  return 'Q';
	case Piece::King:   return 'K';
	default:            return 0; // pawns
	}
}

// You’ll need to implement these if you want actual check/mate detection
bool notation::isCheckAfterMove(const Move& move, const Board& board) {
	Board tempBoard;
	Move moveCopy = move;
	tempBoard.makeMove(moveCopy, tempBoard);

	int enemyColor = tempBoard.colorToMove ^ 1;
	int kingSquare = tempBoard.findKingSquare(enemyColor); // Make sure you have this method
	cout << tempBoard.isSquareAttacked(kingSquare, tempBoard.colorToMove, board) << endl;
	return tempBoard.isSquareAttacked(kingSquare, tempBoard.colorToMove, board);
}

bool notation::isMateAfterMove(const Move& move, const Board& board, moveGenerator gen) {
	Board tempBoard;
	Move moveCopy = move;
	tempBoard.makeMove(moveCopy, tempBoard);

	int enemyColor = tempBoard.colorToMove;

	// Use your MoveGenerator
	vector<Move> legalMoves = gen.GenerateLegalMoves(&tempBoard);

	for (const Move& m : legalMoves) {
		Board testBoard = tempBoard;
		Move testMove = m;
		testBoard.makeMove(testMove, testBoard);

		int kingSquare = testBoard.findKingSquare(enemyColor);
		if (!testBoard.isSquareAttacked(kingSquare, enemyColor ^ 1, board)) {
			cout << "not mate \n";
			return false; // Found a move that avoids checkmate
		}
	}
	cout << "mate \n";
	return true; // No legal moves that avoid check → checkmate
}


string notation::moveToSAN(const Move& move, const Board& board, moveGenerator gen) {
	// Handle castling
	if (move.type == Move::KingsideCastle) return "O-O";
	if (move.type == Move::QueensideCastle) return "O-O-O";

	string san;
	char pieceChar = pieceToChar(move.movedPiece);

	// For pawn moves
	if (move.movedPiece == Piece::Pawn) {
		if (move.type == Move::Capture || move.type == Move::EnPassant) {
			san += 'a' + (move.startSquare % 8); // file of pawn
			san += 'x';
		}
		san += indexToSquare(move.targetSquare);

		if (move.type == Move::Promotion) {
			san += '=';
			san += pieceToChar(move.promotionPiece);
		}
	}
	// For piece moves
	else {
		san += pieceChar;
		if (move.type == Move::Capture) san += 'x';
		san += indexToSquare(move.targetSquare);
	}

	// Append check or mate if implemented
	if (isMateAfterMove(move, board, gen)) {
		san += '#';
	}
	else if (isCheckAfterMove(move, board)) {
		san += '+';
	}

	return san;
}

string notation::toPGN(const vector<Move>& moves, moveGenerator gen) {
	string pgn;
	int moveNumber = 1;

	Board board; // Work on a copy

	for (size_t i = 0; i < moves.size(); ++i) {
		if (i % 2 == 0) {
			pgn += to_string(moveNumber) + ". ";
		}

		string san = moveToSAN(moves[i], board, gen);
		pgn += san + " ";

		Move moveCopy = moves[i];       // 👈 this is the key
		board.makeMove(moveCopy, board);       // now it's happy
		if (i % 2 == 1) {
			moveNumber++;
		}
	}

	return pgn;
}