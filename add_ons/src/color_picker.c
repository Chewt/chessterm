#include <stdio.h>
#include "cursor_move.h"
#include "board.h"

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
        if (square & pawn)
            (square & black) ? printf("\e[30mp") : printf("\e[37mP");
        else if (square & bishop)
            (square & black) ? printf("\e[30mb") : printf("\e[37mB");
        else if (square & knight)
            (square & black) ? printf("\e[30mn") : printf("\e[37mN");
        else if (square & rook)
            (square & black) ? printf("\e[30mr") : printf("\e[37mR");
        else if (square & queen)
            (square & black) ? printf("\e[30mq") : printf("\e[37mQ");
        else if (square & king)
            (square & black) ? printf("\e[30mk") : printf("\e[37mK");
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

/*
  light_squares and dark_squares should be set to the current color values
  between 16 and 255. 
*/

void pick_square_colors(int *light_squares, int *dark_squares, int smol){

  Board board;
  init_board(&board);
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

void read_colors(int *light_color, int *dark_color){
  FILE *fp = fopen("include/settings.h", "r");
  if (fp == NULL){
    fp = fopen("include/settings.h", "w");
    fprintf(fp, "#define SETTINGS\n#define LIGHT 249\n#define DARK 239\n");
    fclose(fp);
    return;
  }
  char buff;
  char light[6] = "LIGHT";
  char dark[5] = "DARK";
  int l=0, d=0;
  while ( (buff = fgetc(fp) )!= EOF){
    if (buff == light[l]){
      l++;
      if (l >= 5){
        fscanf(fp, "%d", light_color);
        l = 0;
      }
    }else{
      l = 0;
    }
    if (buff == dark[d]){
      d++;
      if (d >= 4){
        fscanf(fp, "%d", dark_color);
        d = 0;
      }
    }else{
      d = 0;
    }
  }
  fclose(fp);
}

void write_colors(int light_color, int dark_color){
  FILE *fps = fopen("include/settings.h", "r");
  FILE *fpd = fopen("settings.tmp", "w");

  if (fps == NULL){
    printf("settings.h doesn't exist!\n");
    return;
  }
  if (fpd == NULL){
    printf("No write permissions!\n");
    return;
  }
  char buff;
  char light[14] = "#define LIGHT ";
  char dark[13] = "#define DARK ";
  char header[16] = "#define SETTINGS";
  int l=0, d=0, h=0;
  int temp;
  while ( (buff = fgetc(fps) )!= EOF){
    fputc(buff, fpd);
    if (l < 14){
      if (buff == light[l]){
        l++;
        if (l == 14){
          fscanf(fps, "%d", &temp);
          fprintf(fpd, "%d", light_color);
        }
      }else{
        l = 0;
      }
    }
    if (d < 13){
      if (buff == dark[d]){
        d++;
        if (d == 13){
          fscanf(fps, "%d", &temp);
          fprintf(fpd, "%d", dark_color);
        }
      }else{
        d = 0;
      }
    }
    if (h < 16){
      if (buff == header[h]){
        h++;
      }else{
        h=0;
      }
    }
  }
  fclose(fps);
  fclose(fpd);
  rename("settings.tmp", "include/settings.h");
  if (h < 16){
    FILE *fp = fopen("include/settings.h", "a");
    fprintf(fp, "#define SETTINGS\n");
    fclose(fp);
  }
  if (l < 14){
    FILE *fp = fopen("include/settings.h", "a");
    fprintf(fp, "#define LIGHT %d\n", light_color);
    fclose(fp);
  }
  if (d < 13){
    FILE *fp = fopen("include/settings.h", "a");
    fprintf(fp, "#define DARK %d\n", dark_color);
    fclose(fp);
  }
}

int main(int argc, char **argv){
  int light_color = 249, dark_color = 239;
  read_colors(&light_color, &dark_color);
  pick_square_colors(&light_color, &dark_color, argc > 1);
  write_colors(light_color, dark_color);
  printf("\e[17EDon't forget to run \e[38;5;40m$ make\e[m in your chessterm "
         "directory to update the colors!\n");

  return 0;
}
