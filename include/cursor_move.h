/*
  Cursor_move.h by Zachary Gorman (binarybrain11 on Github)
  A function to handle user input for moving the cursor in the terminal.
  Due to the inclusion of termios.h and unistd.h, this header is 
  limited to Unix systems. 

*/

#ifndef CURSOR_MOVE_H
#define CURSOR_MOVE_H

#include <termios.h>
#include <unistd.h>

/* 
  Custom keyboard bindings if you're a fancy typer
*/

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

/*
  x and y should be passed as the starting position of the cursor and
  will be returned as the current position of the cursor. 
  xmin and xmax (inclusive), ymin and ymax (exclusive) set bounds of x and y.
  xstep and ystep indicate how much to move the cursor for every 
  increment in x and y, respectively. Returns on user hitting the enter key.

  Example: say you want to use the cursor to select a position on a 
  terminal chessboard. Say the chessboard is printed as below. If you 
  send a backspace \b character after printing this board, then the cursor
  will be on the white rook R at 7,7 (if we are 0 indexing). You could pass
  the variables used to increment by reference to the function, or initialize
  two integers to 7 and 7 and pass them by reference. When the function
  returns after the user hits enter, those integers will hold the cursor
  position. For example, if the player entered 4 left arrows and 1 up, 
  those values would now have a 3 and a 6. These values are bound by 
  xmin, ymin, xmax, ymax so if the player tries to move the cursor beyond 
  this boundary, the cursor won't move and x and y won't increment/decrement
  beyond that. The lower boundary is inclusive while the upper is exclusive,
  so for a chessboard, these might be xmin = ymin = 0 and xmax = ymax = 8.
  xstep and ystep are for moving the cursor extra for an increment in x or y.
  The chess board below would have an xstep = 4 and a ystep = 2 because each
  square has 3 horizontal spacers and 1 vertical spacer, so to get to the 
  center of the next square you have to cross the spacers first. 

   r | n | b | q | k | b | n | r
  -------------------------------
   p | p | p | p | p | p | p | p
  -------------------------------
     |   |   |   |   |   |   |
  -------------------------------
     |   |   |   |   |   |   |
  -------------------------------
     |   |   |   |   |   |   |
  -------------------------------
     |   |   |   |   |   |   |
  -------------------------------
   P | P | P | P | P | P | P | P
  -------------------------------
   R | N | B | Q | K | B | N | R


*/

#endif
