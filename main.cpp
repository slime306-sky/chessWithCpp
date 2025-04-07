#include "BoardUI.h"
#include "notation.h"

using namespace std;


class Agent {
private:
	moveGenerator generator;
	vector<Move> legalMoves;

public:

	void printMove(const Move& move) {
		std::string typeStr;
		switch (move.type) {
		case Move::Normal: typeStr = "Normal"; break;
		case Move::Capture: typeStr = "Capture"; break;
		case Move::EnPassant: typeStr = "En Passant"; break;
		case Move::Promotion: typeStr = "Promotion"; break;
		case Move::KingsideCastle: typeStr = "Kingside Castle"; break;
		case Move::QueensideCastle: typeStr = "Queenside Castle"; break;
		default: typeStr = "Unknown"; break;
		}

		std::cout << "Move Details:\n";
		std::cout << "  From Square     : " << move.startSquare << "\n";
		std::cout << "  To Square       : " << move.targetSquare << "\n";
		std::cout << "  Type            : " << typeStr << "\n";
		std::cout << "  Moved Piece     : " << move.movedPiece << "\n";
		std::cout << "  Captured Piece  : " << move.capturedPiece << "\n";
		std::cout << "  Promotion Piece : " << move.promotionPiece << "\n";
	}

	void playRandomMove(Board& board) {
		//cout << "called function";
		legalMoves = generator.GenerateLegalMoves(&board);
		
		// Print all legal moves
		//for (const Move& move : legalMoves) {
		//	//printMove(move);
		//	std::cout << "-------------------\n";
		//}

		int r = rand() % legalMoves.size();
		Move chosen = legalMoves[r];
		//cout << "Bot move: " << chosen.startSquare << " -> " << chosen.targetSquare << " to : " << chosen.promotionPiece << "\n";

		board.makeMove(chosen,board);
		board.recordPosition(board);	
		gameMoves.push_back(chosen);
	}

};

class dataStore {

};

class Game {
private:
	Board board;
	BoardUI boardUi;
	moveGenerator generator;
	int selectedSquare = -1;
	vector<Move> legalMoves;
	bool gameOver = false;
	string resultMessage = "";
	Agent agent;



	int getSquareFromMouse(int x, int y) {
		int file = x / SQUARE_SIZE;
		int rank = 7 - (y / SQUARE_SIZE); // rank 0 at bottom
		return rank * 8 + file;
	}

public:

	Game(SDL_Renderer* renderer) : boardUi(&board) {
		boardUi.loadTextures(renderer);
		board.initZobrist();
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
				// Select a piece if it's your color
				if (Piece::IsColor(clickedPiece, board.colorToMove)) {
					selectedSquare = clickedSquare;
					legalMoves = generator.GenerateLegalMoves(&board);
					//cout << "Selected square: " << selectedSquare << "\nLegal moves:\n";
					for (const auto& m : legalMoves)
						if (m.startSquare == selectedSquare)
							cout << m.startSquare << " -> " << m.targetSquare << endl;
				}
			}
			else {
				bool moved = false;
				for (Move& m : legalMoves) {
					if (m.startSquare == selectedSquare && m.targetSquare == clickedSquare) {
						int movingPiece = board.Square[m.startSquare];

						// Handle promotion
						if (m.type == Move::Promotion && Piece::Type(movingPiece) == Piece::Pawn) {
							int promo = boardUi.showPromotionWindow(renderer, Piece::GetColor(movingPiece));
							m.promotionPiece = promo;
						}

						// Make the move (handles en passant, castling, promotion internally)
						board.makeMove(m,board);
						
						gameMoves.push_back(m);
						moved = true;
						break;
					}
				}

				// Reset selection
				selectedSquare = -1;
				legalMoves.clear();

				if (moved) {
					isGameOver(renderer, board);  // Check for mate/stalemate
					board.recordPosition(board);
					
					//board.printBoard(board);     // Print new board
				}
			}
		}
	}


	void isGameOver(SDL_Renderer* renderer, Board& board) {
		// Generate legal moves for the current position
		vector<Move> nextMoves = generator.GenerateLegalMoves(&board);
		legalMoves = nextMoves; // Update legal moves globally if needed

		int kingSq = board.findKingSquare(board.colorToMove);
		bool inCheck = board.isSquareAttacked(kingSq, Piece::oppositeColor(board.colorToMove), board);

		// Check for checkmate or stalemate
		if (nextMoves.empty()) {
			if (inCheck) {
				gameOver = true;
				cout << "Checkmate! " << ((board.colorToMove == Piece::White) ? "Black" : "White") << " wins!\n";
				string pgn = notation::toPGN(gameMoves, generator);
				cout << "PGN:\n" << pgn << endl;
				return;
			}
			else {
				gameOver = true;
				cout << "Stalemate! It's a draw!\n";
			string pgn = notation::toPGN(gameMoves,generator);
			cout << "PGN:\n" << pgn << endl;
			return;
			}
		}

		// Check for check (not game over, just notify)
		if (inCheck) {
			cout << "Check!\n";
		}

		// 50-move rule (100 halfmoves)
		if (board.halfmoveClock >= 100) {
			gameOver = true;
			cout << "Draw by fifty-move rule!\n";
			string pgn = notation::toPGN(gameMoves, generator);
			cout << "PGN:\n" << pgn << endl;
			return;
		}

		// Insufficient material
		if (board.hasInsufficientMaterial()) {
			gameOver = true;
			cout << "Draw due to insufficient material!\n";
			string pgn = notation::toPGN(gameMoves, generator);
			cout << "PGN:\n" << pgn << endl;
			return;
		}

		//// Threefold repetition
		if (board.isThreefoldRepetition(board)) {
			gameOver = true;
			cout << "Draw by threefold repetition!\n";
			string pgn = notation::toPGN(gameMoves,generator);
			cout << "PGN:\n" << pgn << endl;
			return;
		}
	}


	void update(SDL_Renderer* renderer) {

		if (board.colorToMove == Piece::Black && !gameOver) {
			agent.playRandomMove(board);
			isGameOver(renderer,board);
			//board.printBoard(board);
		}
		else if(board.colorToMove == Piece::White && !gameOver) {
			agent.playRandomMove(board);
			isGameOver(renderer, board);
			//board.printBoard(board);
		}


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


		game.update(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}