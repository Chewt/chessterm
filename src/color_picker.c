#include <stdio.h>
#include "cursor_move.h"
#include "board.h"
#include "config.h"

/*
 * Print modified from board.c to take color arguments that are normally
 * defined as preprocessor constants rather than variables.
 */

void print_example_board(Board* board, int LIGHT, int DARK)
{
    int i;
    printf("\u2554");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u2557");
    for (i = 0; i < 64; ++i)
    {
        uint8_t square = board->position[i];
        if (i % 8 == 0)
            printf("\n\u2551");
        /* Bullshit that colors them checkered-like
         * - Courtesy of Zach Gorman
         */
        if (!(!(i & 1) ^ !(i & 8)))
            printf("\e[48;5;%dm", LIGHT);
        else
            printf("\e[48;5;%dm", DARK);
        printf(" ");
        if (square & PAWN)
            (square & BLACK) ? printf("\e[30mp") : printf("\e[37mP");
        else if (square & BISHOP)
            (square & BLACK) ? printf("\e[30mb") : printf("\e[37mB");
        else if (square & KNIGHT)
            (square & BLACK) ? printf("\e[30mn") : printf("\e[37mN");
        else if (square & ROOK)
            (square & BLACK) ? printf("\e[30mr") : printf("\e[37mR");
        else if (square & QUEEN)
            (square & BLACK) ? printf("\e[30mq") : printf("\e[37mQ");
        else if (square & KING)
            (square & BLACK) ? printf("\e[30mk") : printf("\e[37mK");
        else
            printf(" ");
        printf(" \e[0m");

        if (i % 8 == 7)
            printf("\u2551");
        else
            printf("\u2502");
        if (i % 8 == 7 && i / 8 != 7)
        {
            printf("\n\u2551\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c"
                   "\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500"
                   "\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500\u2500"
                   "\u2500\u253c\u2500\u2500\u2500\u2551");
        }
    }
    printf("\n");
    printf("\u255a");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u255d");
    printf("\n");
}

void write_colors(int light_color, int dark_color){
    printf("Add the following lines to your config file:\n");
    printf("%s%d\n", CONFIG_STR_LIGHT_COLOR, light_color);
    printf("%s%d\n", CONFIG_STR_DARK_COLOR, dark_color);
}

void move_cursor(int *x, int *y, int xmin, int xmax, int ymin, int ymax, 
                 int xstep, int ystep){

  /* Start termios input high-jacking */
  struct termios told, tnew;
  tcgetattr(STDIN_FILENO, &told);
  tnew = told;
  /* Turns on canonical input, turns off echo */
  tnew.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &tnew);

  char ch[] = "0";
  while (*ch != '\n' && *ch != '\r'){ 	/* \r might not matter */
    scanf("%c", ch);
    switch(*ch){
      case '\t':
        if (++*x >= xmax){
          *x = xmin;
          if (++*y >= ymax){
            printf("\e[%dA", ystep);
            *y=ymin;
          }else{
            printf("\e[%dB", ystep);
          }
        }else{
          printf("\e[%dC", xstep);
        }
        break;

      // Arrow keys
      case '\e':
        scanf("%c", ch);
        if (*ch == '['){
          scanf("%c", ch);
          switch(*ch){

            case 'D':
              if (--*x < xmin)
                ++*x;
              else
                printf("\e[%dD", xstep);
              break;

            case 'A':
              if (--*y < ymin)
                ++*y;
              else
                printf("\e[%dA", ystep);
              break;

            case 'B':
              if (++*y >= ymax)
                --*y;
              else
                printf("\e[%dB", ystep);
              break;

            case 'C':
              if (++*x >= xmax)
                --*x;
              else
                printf("\e[%dC", xstep);
              break;

            default:
              break;
          }
        }
        break;

      case LEFT:
        if (--*x < xmin)
          ++*x;
        else
          printf("\e[%dD", xstep);
        break;

      case UP:
        if (--*y < ymin)
          ++*y;
        else
          printf("\e[%dA", ystep);
        break;

      case DOWN:
        if (++*y >= ymax)
          --*y;
        else
          printf("\e[%dB", ystep);
        break;

      case RIGHT:
        if (++*x >= xmax)
          --*x;
        else
          printf("\e[%dC", xstep);
        break;

      default:
        break;
    }
  }
  // Puts terminal input settings back to normal
  tcsetattr(STDIN_FILENO, TCSANOW, &told);
}

/*
  light_squares and dark_squares should be set to the current color values
  between 16 and 255.
*/

void pick_square_colors(int *light_squares, int *dark_squares, int smol){

  Board board;
  board.notes = NULL;
  default_board(&board);
  int square = 0;
  while(1==1){
    printf("Use arrow keys or wasd to navigate and enter to select.\nSelect the color for ");
    if (square)
      printf("light");
    else
      printf("dark");
    printf(" squares   \n");
    for (int bright = 0; bright < 6; bright++){
      for (int colo = 0; colo < 36; colo++){
        printf("\e[48;5;%dm", 16 + colo + 36*bright);
        if (16 + colo + 36*bright == *light_squares)
          printf("\e[30mL\e[0m");
        else if (16 + colo + 36*bright == *dark_squares)
          printf("\e[37mD\e[0m");
        else
          printf(" ");
        printf("\e[0m");
      }
      printf("\n");
    }
    for (int grey = 232; grey < 256; grey++){
      printf("\e[48;5;%dm", grey);
      if (grey == *light_squares)
        printf("\e[30mL\e[0m");
      else if (grey == *dark_squares)
        printf("\e[37mD\e[0m");
      else
        printf(" ");
    }
    printf("\e[0m  --Quit--\n");
    if (!smol)
      print_example_board(&board, *light_squares, *dark_squares);
    printf("\e[%dA", 7 + 17*!smol);
    int x = 0, y = 0;
    move_cursor(&x, &y, 0, 36, 0, 7, 1, 1);
    if (x > 24 && y > 5){
      printf("\n");
      return;
    }
    if (y >= 0)
      printf("\e[%dA", y+1);
    if (x > 0)
      printf("\e[%dD", x);
    if (square){
      *light_squares = 16 + x + 36*y;
    }else{
      *dark_squares = 16 + x + 36*y;
    }
    square = !square;
    printf("\e[F");
  }
  //printf("\e[%dE", 7);
}

void move_cursor_basic(){
  int a = 0;
  int b = 0;
  move_cursor(&a,&b,0,100,0,100,1,1);
}

int color_picker(int argc, char **argv){
  int light_color = 249, dark_color = 239;
  pick_square_colors(&light_color, &dark_color, 0);
  write_colors(light_color, dark_color);
  printf("\e[17EDon't forget to run \e[38;5;40m$ make\e[m in your chessterm "
         "directory to update the colors!\n");

  return 0;
}
