
JungleChess/DouShouQi written in C++ using SFML library for (simple) graphics & arial.ttf monospace font for the pieces.

Contains an alpha-beta searcher with move-ordering, piece-square-tables, transposition hashtables and unlimited Undo/Redo functionality.


**Starting options:**

-? , -h , --help : for help-text output

-d : full debug mode

-n : no-output mode (show only errors)

[no flag] : default output (reduced debug infos)

--depth [number] : search-depth in plies

--setup : start game in *Setup Mode*


**Keys during game:**

[left mouse button] : move pieces

[escape] : Quit after confirmation

[S] : save complete game to program directory (probably "../JungleChess/build")

[L] : load saved game from program directory (if exists)

[Backspace] : take back the last ply (half-move)

[Shift-Backspace] : undo take-back of last ply

[P] : Switch between three different piece-sets

[G] : start AI  (if it was its turn to move *or* at the start of the game/after setup)

[R] : rotate the board by 180°


(The computer plays red, from the bottom. The player always starts with blue.)


----


Compilation is easiest under Linux (Mint). You'll need these:
- a build-environment (GCC, CMake, etc.)
- SFML library (for graphics)
- arial.ttf (monospace)


**Instructions:**

First, install CMake and the necessary build-environment (ask ChatGPT/Gemini for the exact details, if necessary)
then download the master-zip (click on that big green button), or use *Git* to clone the repo

After download, CD into the "JungleChess" folder/directory, create subDir with "mkdir build".

CD into that *build*-directory/folder.

Then give command "cmake .." to create all necessary make-files.

Then, a "make" should compile & link everything together.

Or (if you have Linux): just download the "jungle_chess" linux binary + assets/arial.ttf  (you might need to install SFML library in that scenario, i didn't test that)

Have fun!

-----

Reference:
https://en.wikipedia.org/wiki/Jungle_(board_game)

When started, the program used to look like this (now can be switched to, with 'P'):
![Jungle board](https://github.com/JSettler/JungleChess/blob/master/jungle_chess.png)

Now with alternate piece-sets, this is now the default :
![Jungle board](https://github.com/JSettler/JungleChess/blob/master/jungle-chess.png)

Setup-mode added:
![Jungle board](https://github.com/JSettler/JungleChess/blob/master/jungle_setup-mode.png)

-----

social media:

https://discord.gg/257jDH3nmn  - our Dou Shou Qi/Jungle chess community


