# ♟️ chessWithCpp

A C++ Chess game built with SDL — for people who love 64 squares and a bit of code magic.

## 🎮 Features

- Fully playable 2-player chess
- Supports:
  - Castling
  - En Passant
  - Promotions
- Move validation
- Precomputed move data (it's fast, yo!)
- Clean UI using SDL (Simple DirectMedia Layer)
- Modular code with clear class separation

## 🛠️ Requirements

- C++17 (or newer)
- [SDL3](https://github.com/libsdl-org/SDL) (Simple DirectMedia Layer)
- A brain (optional but helpful)

## 🧑‍💻 Build Instructions

### 🐧 Linux / 🪟 Windows (via MinGW or WSL) / 🍏 macOS

```bash
git clone https://github.com/YOUR_USERNAME/chessWithCpp.git
cd chessWithCpp
g++ -std=c++17 -I./ -lSDL3 *.cpp -o chess
./chess
```

### Note:
Make sure SDL3 is installed and accessible in your system path or linked properly during compilation. If you see weird undefined reference errors… it’s probably SDL throwing a tantrum.

## 🗂️ Project Structure

```
├── Board.cpp / .h            # Chess board logic
├── Piece.cpp / .h            # Base class for all chess pieces
├── Renderer.cpp / .h         # Handles SDL rendering
├── BoardUI.cpp / .h          # User Interface glue code
├── PrecomputedMoveData.*     # For optimized move validation
├── gameLogic.cpp             # Game loop and logic
├── gameUI.cpp                # UI setup & input handling
├── main.cpp                  # The one ring that rules them all
```

## 🤖 Agent Class

Yeah, there’s a stub of an "Agent" class in `main.cpp` for printing move info. You could totally hook up an AI or stockfish here if you’re feeling spicy.

## 📦 Precompiled Binaries

If you just wanna play (no coding jazz), download the precompiled version here:

👉 [Chess.exe download](https://github.com/slime306-sky/chessWithCppEXE.git)

Put the following files in the same folder:

```
- chess.exe
- sdl3.dll
- assets/
```

Then run it. Like magic. ✨

## 🧠 Future Improvements (maybe?)

- AI opponent using Minimax or Alpha-Beta
- Move history / Undo
- Highlight legal moves
- Sound effects!
- Online multiplayer? (a coder can dream...)

## 🙏 Credits

Created with coffee & love by slime306-sky

Wanna use this? Go ahead! A shoutout would be super nice. 💖
