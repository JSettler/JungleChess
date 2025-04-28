
JungleChess/DouShouQi written in C++ using SFML library for (simple) graphics & arial.ttf monospace font for the pieces.

Contains an alpha-beta searcher with move-ordering, piece-square-tables, hashing and unlimited Undo/Redo functionality.


Starting options:

-? , -h , --help : for help-text output

-d : full debug mode

-n : no-output mode (show only errors)

[no flag] : default output (reduced debug infos)

--depth [number] : search-depth in plies


Keys during game:
[left mouse button] : move pieces
[escape] : Quit immediately
[S] : save complete game to program directory (probably "../JungleChess/build")
[L] : load saved game from program directory (if exists)
[Backspace] : undo full move (2 plies)
[Shift-Backspace] : redo full move


Compilation is easiest under Linux (Mint). You'll need these:
- a build-environment (GCC, CMake, etc.)
- SFML library (for graphics)
- arial.ttf (monospace)


Have fun!


