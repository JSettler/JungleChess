
JungleChess/DouShouQi written in C++ using SFML library for (simple) graphics & arial.ttf monospace font for the pieces.

Contains an alpha-beta searcher with move-ordering, piece-square-tables, transposition hashtables and unlimited Undo/Redo functionality.


**Starting options:**

-? , -h , --help : for help-text output

-d : full debug mode

-n : no-output mode (show only errors)

[no flag] : default output (reduced debug infos)

--depth [number] : search-depth in plies


**Keys during game:**

[left mouse button] : move pieces

[escape] : Quit immediately

[S] : save complete game to program directory (probably "../JungleChess/build")

[L] : load saved game from program directory (if exists)

[Backspace] : undo full move (2 plies)

[Shift-Backspace] : redo full move

[P] : Switch between three different piece-sets


(The computer plays red, from the bottom. The player always starts with blue.)

----

Compilation is easiest under Linux (Mint). You'll need these:
- a build-environment (GCC, CMake, etc.)
- SFML library (for graphics)
- arial.ttf (monospace)


**Instructions:**

First, install CMake and the necessary build-environment (ask ChatGPT/Gemini for the exact details, if necessary)
then download the master-zip (click on that big green button)

After download, CD into the "JungleChess" folder/directory, create subDir with "mkdir build".

CD into that *build*-directory/folder.

Then give command "cmake .." to create all necessary make-files.

Then, a "make" should compile & link everything together.

Have fun!

-----

Reference:
https://en.wikipedia.org/wiki/Jungle_(board_game)

When started, the program used to look like this (now can be switched to, with 'P'):
![Jungle board](https://github.com/JSettler/JungleChess/blob/master/jungle_chess.png)

Now with alternate piece-sets, this is now the default :
![Jungle board](https://github.com/JSettler/JungleChess/blob/master/jungle-chess.png)

