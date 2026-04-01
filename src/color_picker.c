#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cursor_move.h"
#include "board.h"
#include "io.h"
#include "config.h"

void write_colors(Config* config){
    // Clear screen
    Board board;
    board.notes = NULL;
    default_board(&board);
    printf("\e[2J\e[H");
    print_fancy(&board, config);
    printf("Add the following lines to your config file:\n");
    printf("%s%d\n", CONFIG_STR_COLOR_MODE, config->color_mode);
    printf("%s%d\n", CONFIG_STR_LIGHT_COLOR, config->board_color_light);
    printf("%s%d\n", CONFIG_STR_DARK_COLOR, config->board_color_dark);
    const char* art_config_str;
    switch (config->piece_art) {
        case UNICODE:
            art_config_str = PIECE_ART_UNICODE;
            break;
        case BRAILLE:
            art_config_str = PIECE_ART_BRAILLE;
            break;
        case ASCII:
        default:
            art_config_str = PIECE_ART_ASCII;
            break;
    }
    printf("%s%s\n", CONFIG_STR_PIECE_ART, art_config_str);
}

enum SelectionResult {
    ACCEPT,
    QUIT,
    SWITCH_COLOR_MODE,
    PIECE_SELECT
};

int move_cursor(int *x, int *y, int xmin, int xmax, int ymin, int ymax, 
                 int xstep, int ystep){

  /* Start termios input high-jacking */
  struct termios told, tnew;
  tcgetattr(STDIN_FILENO, &told);
  tnew = told;
  /* Turns on canonical input, turns off echo */
  tnew.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &tnew);

  char ch[] = "0";
  while (*ch != '\n' && // Select under cursor
         *ch != '\r' && // same as above, \r might not matter
         *ch != ' '  && // Switch color mode
         *ch != 'p'  && // Switch to piece mode
         *ch != 'q'     // Cancel
         ){ 	
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
  if (*ch == ' ') {
      return SWITCH_COLOR_MODE;
  }
  else if (*ch == 'q') {
      return QUIT;
  }
  else if (*ch == 'p') {
      return PIECE_SELECT;
  }
  return ACCEPT;
}

int pick_piece_style() {
    // Clear screen
    printf("\e[2J\e[H");
    printf("Select a piece style by pressing the corresponding number\n");
    // ASCII,
    // UNICODE,
    // NUM_ART_STYLES
    for (int i = 0; i < NUM_ART_STYLES; ++i) {
        // void print_piece(uint8_t piece, int style)
        printf("\e[1B%d\e[1A", i + 1);
        print_piece(PAWN  , i);
        print_piece(BISHOP, i);
        print_piece(KNIGHT, i);
        print_piece(ROOK  , i);
        print_piece(KING  , i);
        print_piece(QUEEN , i);
        printf("\e[3E");
    }
    char ch[256];
    fgets(ch, 256, stdin);
    int selection = strtol(ch, NULL, 10);
    return selection - 1;
}

/*
  light_squares and dark_squares should be set to the current color values
  between 16 and 255.
*/

void pick_square_colors(Config* config, int smol){
  Board board;
  board.notes = NULL;
  default_board(&board);
  int square = 0;

  Config temp_config;
  memcpy(&temp_config, config, sizeof(*config));

  int maxx, maxy;

  while(1){
      printf("\e[2J\e[H");
      const char *message = 
          "Use arrow keys or wasd to navigate and enter to select.\n"
          "Select the color for %s squares.\n"
          "'q' to cancel, SPACE to switch color modes, press 'p' to switch piece styles.\n";
      printf(message, (square) ? "light" : "dark" );
      if (temp_config.color_mode == 16) {
          maxx = 20;
          maxy = 1;
          // Print 16 color pallet
          for (int colo = 40; colo < 48; colo++){
              printf("\e[%dm", colo);
              if (colo == temp_config.board_color_light)
                  printf("\e[30mL\e[0m");
              else if (colo == temp_config.board_color_dark)
                  printf("\e[37mD\e[0m");
              else
                  printf(" ");
              printf("\e[0m");
          }
      } else {
          maxx = 36;
          maxy = 7;
          // Print 256 color pallet
          for (int bright = 0; bright < 6; bright++){
              for (int colo = 0; colo < 36; colo++){
                  printf("\e[48;5;%dm", 16 + colo + 36*bright);
                  if (16 + colo + 36*bright == temp_config.board_color_light)
                      printf("\e[30mL\e[0m");
                  else if (16 + colo + 36*bright == temp_config.board_color_dark)
                      printf("\e[37mD\e[0m");
                  else
                      printf(" ");
                  printf("\e[0m");
              }
              printf("\n");
          }
          for (int grey = 232; grey < 256; grey++){
              printf("\e[48;5;%dm", grey);
              if (grey == temp_config.board_color_light)
                  printf("\e[30mL\e[0m");
              else if (grey == temp_config.board_color_dark)
                  printf("\e[37mD\e[0m");
              else
                  printf(" ");
          }
      }
      printf("\e[0m  --Quit--\n");
      if (!smol)
          print_fancy(&board, &temp_config);

      // Get number of lines of message and move down by that many.
      int message_line_count = 0;
      for (size_t i = 0; i < strlen(message); i++) {
          if (message[i] == '\n')
              message_line_count++;
      }
      printf("\e[H\e[%dB", message_line_count);

      int x = 0, y = 0;
      int result = move_cursor(&x, &y, 0, maxx, 0, maxy, 1, 1);
      switch (result) {
          case SWITCH_COLOR_MODE:
              temp_config.color_mode = (temp_config.color_mode == 16) ? 256 : 16;
              if (temp_config.color_mode == 16) {
                  temp_config.board_color_light = DEFAULT_LIGHT_COLOR_16;
                  temp_config.board_color_dark = DEFAULT_DARK_COLOR_16;
              } else {
                  temp_config.board_color_light = DEFAULT_LIGHT_COLOR_256;
                  temp_config.board_color_dark = DEFAULT_DARK_COLOR_256;
              }
              break;
          case PIECE_SELECT:
              temp_config.piece_art = pick_piece_style();
              break;
          case QUIT:
              return;
          case ACCEPT:
              if (x > (maxx - 11) && y == (maxy - 1)){
                  printf("\n");
                  memcpy(config, &temp_config, sizeof(*config));
                  return;
              }
              if (square){
                  temp_config.board_color_light = (temp_config.color_mode == 256) ? 16 + x + 36*y : 40 + x;
              }else{
                  temp_config.board_color_dark = (temp_config.color_mode == 256) ? 16 + x + 36*y : 40 + x;
              }
              square = !square;
              printf("\e[F");
              break;
      }
  }
}

int color_picker(Config* config){
    pick_square_colors(config, 0);
    write_colors(config);
    return 0;
}
