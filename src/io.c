#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <wchar.h>
#include "io.h"
#include "board.h"
#include "settings.h"

#ifndef SETTINGS
#define LIGHT 179
#define DARK  58
#endif

#define BRIGHT_WHITE 15

/* Prints information about the current position to the screen.
 * It will print the following information:
 * - Who's turn it is.
 * - Castling rights.
 * - Current en pessant location.
 * - Current halfmove counter.
 * - Current fullmove counter.
 * - Position of the black king.
 * - Position of the white king.
 */
void board_stats(Board *board) 
{
  int chars_printed = 0;
  if (board->to_move == 1) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "Black");
  } else if (board->to_move == 0) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "White");
  }
  chars_printed += snprintf(board->notes + chars_printed,
               NOTES_LENGTH - strlen(board->notes), " to move.\nCastling: ");
  if (board->castling & 0x08) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "K");
  }
  if (board->castling & 0x04) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "Q");
  }
  if (board->castling & 0x02) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "k");
  }
  if (board->castling & 0x01) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "q");
  }
  chars_printed +=
      snprintf(board->notes + chars_printed,
               NOTES_LENGTH - strlen(board->notes), "\nEn pessant: ");
  if (board->en_p == -1) {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "None\n");
  } else {
    chars_printed += snprintf(board->notes + chars_printed,
                              NOTES_LENGTH - strlen(board->notes), "%c%d\n",
                              (board->en_p % 8) + 'a', 8 - (board->en_p / 8));
  }
  chars_printed += snprintf(
      board->notes + chars_printed, NOTES_LENGTH - strlen(board->notes),
      "Halfmoves: %d\nFullmoves: %d\n", board->halfmoves, board->moves);
  chars_printed +=
      snprintf(board->notes + chars_printed,
               NOTES_LENGTH - strlen(board->notes), "Whiteking_pos: %c%d\n",
               (board->wking_pos % 8) + 'a', 8 - (board->wking_pos / 8));
  chars_printed +=
      snprintf(board->notes + chars_printed,
               NOTES_LENGTH - strlen(board->notes), "Blackking_pos: %c%d\n",
               (board->bking_pos % 8) + 'a', 8 - (board->bking_pos / 8));
}

/* Prints the board to the screen */
void print_board(Board* board)
{
    int i;
    printf("  \u2554");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u2557");
    for (i = 0; i < 64; ++i)
    {
        uint8_t square = board->position[i];
        if (i % 8 == 0)
            printf("\n%d \u2551", 8 - (i / 8));
        /* Bullshit that colors them checkered-like 
         * - Courtesy of Zach Gorman
         */
        if (!(!(i & 1) ^ !(i & 8))) 
            printf("\e[48;5;%dm", LIGHT);
        else
            printf("\e[48;5;%dm", DARK);
        printf(" \e[1m");
        if (square & PAWN)
            (square & BLACK) ? printf("\e[38;5;232mp") : printf("\e[37mP");
        else if (square & BISHOP)
            (square & BLACK) ? printf("\e[38;5;232mb") : printf("\e[37mB");
        else if (square & KNIGHT)
            (square & BLACK) ? printf("\e[38;5;232mn") : printf("\e[37mN");
        else if (square & ROOK)
            (square & BLACK) ? printf("\e[38;5;232mr") : printf("\e[37mR");
        else if (square & QUEEN)
            (square & BLACK) ? printf("\e[38;5;232mq") : printf("\e[37mQ");
        else if (square & KING)
            (square & BLACK) ? printf("\e[38;5;232mk") : printf("\e[37mK");
        else
            printf(" ");
        printf(" \e[0m");

        if (i % 8 == 7)
            printf("\u2551");
        else
            printf("\u2502");
        if (i % 8 == 7 && i / 8 != 7)
        {
            printf("\n  \u2551\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c"
                    "\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500"
                    "\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500\u2500"
                    "\u2500\u253c\u2500\u2500\u2500\u2551");
        }
    }
    printf("\n  ");
    printf("\u255a");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u255d\n ");
    for (i = 0; i < 8; i++)
        printf("   %c", 'a' + i);
    printf("\n");
}

enum {
    LIGHT_SQUARE = 1,
    DARK_SQUARE,
    LIGHT_PIECE_LIGHT,
    LIGHT_PIECE_DARK,
    DARK_PIECE_LIGHT,
    DARK_PIECE_DARK
};

/* Prints the ascii representation of the passed piece to the board. It will
 * take up 7 collumns and 3 rows
 */
void mvprint_piecew(int y, int x, uint8_t piece)
{
    attron(A_BOLD);
    if (piece & PAWN)
    {
        mvaddstr(y    , x, "   O   ");
        mvaddstr(y + 1, x, "  ( )  ");
        mvaddstr(y + 2, x, "  ===  ");
    }
    else if (piece & BISHOP)
    {
        mvaddstr(y    , x, "  (/)  ");
        mvaddstr(y + 1, x, "  / \\  ");
        mvaddstr(y + 2, x, "  ===  ");
    }
    else if (piece & KNIGHT)
    {
        mvaddstr(y    , x, "  <*^  ");
        mvaddstr(y + 1, x, "  / |  ");
        mvaddstr(y + 2, x, "  ===  ");
    }
    else if (piece & ROOK)
    {
        mvaddstr(y    , x, "  ooo  ");
        mvaddstr(y + 1, x, "  | |  ");
        mvaddstr(y + 2, x, "  ===  ");
    }
    else if (piece & KING)
    {
        mvaddstr(y    , x, "  ^+^  ");
        mvaddstr(y + 1, x, "  )|(  ");
        mvaddstr(y + 2, x, "  ===  ");
    }
    else if (piece & QUEEN)
    {
        mvaddstr(y    , x, "  oOo  ");
        mvaddstr(y + 1, x, "  )|(  ");
        mvaddstr(y + 2, x, "  ===  ");
    }
    else
    {
        mvaddstr(y    , x, "       ");
        mvaddstr(y + 1, x, "       ");
        mvaddstr(y + 2, x, "       ");
    }
    attroff(A_BOLD);
}

void print_boardw(Board* board)
{
    // Initialize colors needed for the board
    start_color();

    init_color(BRIGHT_WHITE, 1000, 1000, 1000);
    init_pair(LIGHT_SQUARE,      COLOR_YELLOW, COLOR_YELLOW);
    init_pair(DARK_SQUARE ,      COLOR_GREEN, COLOR_GREEN);
    init_pair(LIGHT_PIECE_LIGHT, BRIGHT_WHITE, COLOR_YELLOW);
    init_pair(LIGHT_PIECE_DARK,  BRIGHT_WHITE, COLOR_GREEN );
    init_pair(DARK_PIECE_LIGHT,  COLOR_BLACK, COLOR_YELLOW);
    init_pair(DARK_PIECE_DARK,   COLOR_BLACK, COLOR_GREEN );
    int x_pos = 0;
    int y_pos = 0;

    // Determine where the board should be placed on the screen
    int width_needed = 7 * 8 + 12;
    int start_xpos = 0;
    int max_x = getmaxx(stdscr);
    if ((max_x / 2) >= (width_needed / 2)) 
        x_pos = start_xpos = (max_x / 2) - (width_needed / 2);

    // Set screen to blank
    clear();
    
    // Set up fancy border
    mvaddstr(y_pos, x_pos, "   ╔════════════════════════════════════════════════════════╗");
    y_pos++;
    int i;
    int nums = 8;
    for (i = 0; i < 24; ++i)
    {
        if (!((i + 2) % 3))
        {
            mvaddch(y_pos + i, x_pos, '0' + nums--);
        }
        mvaddstr(y_pos + i, x_pos + 1, "  ║                                                        ║");
    }
    mvaddstr(y_pos + 24, x_pos, "   ╚════════════════════════════════════════════════════════╝");
    mvaddstr(y_pos + 25, x_pos, "       a      b      c      d      e      f      g      h    ");
    x_pos += 4;

    // Print pieces
    int x_edge = x_pos;
    for (i = 0; i < 64; ++i)
    {
        if ((i + (i / 8)) % 2 == 0)
        {
            if (board->position[i] == 0)
                attrset(COLOR_PAIR(LIGHT_SQUARE));
            else if (board->position[i] & BLACK)
                attrset(COLOR_PAIR(DARK_PIECE_LIGHT));
            else
                attrset(COLOR_PAIR(LIGHT_PIECE_LIGHT));
        }
        else
        {
            if (board->position[i] == 0)
                attrset(COLOR_PAIR(DARK_SQUARE));
            else if (board->position[i] & BLACK)
                attrset(COLOR_PAIR(DARK_PIECE_DARK));
            else
                attrset(COLOR_PAIR(LIGHT_PIECE_DARK));
        }
        if (i && (i % 8 == 0))
        {
            y_pos += 3;
            x_pos = x_edge;
        }
        mvprint_piecew(y_pos, x_pos, board->position[i]);
        x_pos += 7;
    }
    move(y_pos + 5, x_edge - 1);
    attrset(COLOR_PAIR(LIGHT_SQUARE));
    attroff(COLOR_PAIR(LIGHT_SQUARE));
}

void print_notesw(Board* board)
{
    // Count how many lines the notes have
    char c;
    int i = 0;
    int linecount = 0;
    while ((c = board->notes[i++]) != '\0')
    {
        if (c == '\n')
            linecount++;
    }

    // Print notes and adjust starting y for notes
    int x, y;
    x = 0;
    y = getcury(stdscr);
    if ((getmaxy(stdscr) - y) <= linecount)
        y -= linecount;
    i = 0;
    while ((c = board->notes[i++]) != '\0')
    {
        if (c == '\n')
            move(++y, x = 0);
        else
            mvaddch(y, x++, c);
    }

    // Reset notes to empty
    board->notes[0] = '\0';
}

/* Prints the ascii representation of the passed piece to the board. It will
 * take up 5 collumns and 3 rows
 */
void print_piece(uint8_t piece)
{
    if (piece & PAWN)
    {
        printf("   O   \e[B\e[7D"
               "  ( )  \e[B\e[7D"
               "  ===  \e[2A");
    }
    else if (piece & BISHOP)
    {
        printf("  (/)  \e[B\e[7D"
               "  / \\  \e[B\e[7D"
               "  ===  \e[2A");
    }
    else if (piece & KNIGHT)
    {
        printf("  <*^  \e[B\e[7D"
               "  / |  \e[B\e[7D"
               "  ===  \e[2A");
    }
    else if (piece & ROOK)
    {
        printf("  ooo  \e[B\e[7D"
               "  | |  \e[B\e[7D"
               "  ===  \e[2A");
    }
    else if (piece & KING)
    {
        printf("  ^+^  \e[B\e[7D"
               "  )|(  \e[B\e[7D"
               "  ===  \e[2A");
    }
    else if (piece & QUEEN)
    {
        printf("  oOo  \e[B\e[7D"
               "  )|(  \e[B\e[7D"
               "  ===  \e[2A");
    }
    else
        printf("       \e[B\e[7D"
               "       \e[B\e[7D"
               "       \e[2A");
}

/* Prints the board using ascii pieces and shows the current names of the 
 * players along with their material scores.
 */
void print_fancy(Board* board)
{
    int i;
    printf("   \u2554");
    for (i = 0; i < 56; ++i)
        printf("\u2550");
    printf("\u2557");
    int white_score[6];
    int black_score[6];
    const char* piece_chars = "pbnrq";
    get_material_scores(board, white_score, black_score);
    printf(" %s: ", board->black_name);
    if (white_score[0] - black_score[0] < 0)
        printf("%+d ", black_score[0] - white_score[0]);
    int j;
    for (i = 1; i < 6; ++i)
        for (j = 0; j < white_score[6 - i]; ++j)
            printf("%c", piece_chars[5 - i] - 32);
    printf("\n");
    
    for (i = 0; i < 24; ++i)
    {
        if (i % 3 == 1)
            printf(" %d \u2551", 8 - (i / 3));
        else
            printf("   \u2551");
        int j;
        for (j = 0; j < 8; ++j)
            printf("       ");
        printf("\u2551\n");
    }
    printf("   \u255a");
    for (i = 0; i < 56; ++i)
        printf("\u2550");
    printf("\u255d");
    printf(" %s: ", board->white_name);
    if (white_score[0] - black_score[0] > 0)
        printf("%+d ", white_score[0] - black_score[0]);
    for (i = 1; i < 6; ++i)
        for (j = 0; j < black_score[6 - i]; ++j)
            printf("%c", piece_chars[5 - i]);
    printf("\n       ");
    for (i = 0; i < 8; ++i)
        printf("%c      ", 'a' + i);
    printf("\n");

    printf("\e[26F\e[4C");

    Move prev_move;
    if (board->history_count > 0)
        prev_move = board->history[board->history_count - 1];
    else 
        prev_move = board->history[board->history_count];
    int8_t prev_src = prev_move.src_rank * 8 + prev_move.src_file;
    int8_t prev_dest = prev_move.dest;

    for (i = 0; i < 64; ++i)
    {
        /* Bullshit that colors them checkered-like 
         * - Courtesy of Zach Gorman
         */
        uint8_t square = board->position[i];
        if (prev_dest != -1 && (i == prev_src || i == prev_dest))
        {
            if (i == prev_src)
                printf("\e[48;5;6m");
            if (i == prev_dest)
                printf("\e[48;5;12m");
        }
        else
        {
            if (!(!(i & 1) ^ !(i & 8))) 
                printf("\e[48;5;%dm", LIGHT);
            else
                printf("\e[48;5;%dm", DARK);
        }

        printf("\e[1m");
        (square & BLACK) ? printf("\e[38;5;232m") : printf("\e[37m");
        print_piece(square);
        printf("\e[0m");

        if (i % 8 == 7)
            printf("\u2551\e[3E\e[4C");
    }
    printf("\e[1E\n");
}

/* Prints the fancy version of the board, but from black's perspective */
void print_fancy_flipped(Board* board)
{
    int i;

    printf("   \u2554");
    for (i = 0; i < 56; ++i)
        printf("\u2550");
    printf("\u2557");
    int white_score[6];
    int black_score[6];
    const char* piece_chars = "pbnrq";
    get_material_scores(board, white_score, black_score);
    printf(" %s: ", board->white_name);
    if (white_score[0] - black_score[0] > 0)
        printf("%+d ", white_score[0] - black_score[0]);
    int j;
    for (i = 1; i < 6; ++i)
        for (j = 0; j < black_score[6 - i]; ++j)
            printf("%c", piece_chars[5 - i]);
    printf("\n");
    for (i = 0; i < 24; ++i)
    {
        if (i % 3 == 1)
            printf(" %d \u2551", (i / 3) + 1);
        else
            printf("   \u2551");
        int j;
        for (j = 0; j < 8; ++j)
            printf("       ");
        printf("\u2551\n");
    }
    printf("   \u255a");
    for (i = 0; i < 56; ++i)
        printf("\u2550");
    printf("\u255d");
    printf(" %s: ", board->black_name);
    if (white_score[0] - black_score[0] < 0)
        printf("%+d ", black_score[0] - white_score[0]);
    for (i = 1; i < 6; ++i)
        for (j = 0; j < white_score[6 - i]; ++j)
            printf("%c", piece_chars[5 - i] - 32);
    printf("\n       ");
    for (i = 0; i < 8; ++i)
        printf("%c      ", 'h' - i);
    printf("\n");

    printf("\e[26F\e[4C");

    Move prev_move;
    if (board->history_count > 0)
        prev_move = board->history[board->history_count - 1];
    else 
        prev_move = board->history[board->history_count];
    int8_t prev_src = 63 - (prev_move.src_rank * 8 + prev_move.src_file);
    int8_t prev_dest = 63 - prev_move.dest;

    for (i = 0; i < 64; ++i)
    {
        /* Bullshit that colors them checkered-like 
         * - Courtesy of Zach Gorman
         */
        uint8_t square = board->position[63 - i];
        if (prev_dest != -1 && (i == prev_src || i == prev_dest))
        {
            if (i == prev_src)
                printf("\e[48;5;6m");
            if (i == prev_dest)
                printf("\e[48;5;12m");
        } 
        else 
        {
            if (!(!(i & 1) ^ !(i & 8)))
                printf("\e[48;5;%dm", LIGHT);
            else
                printf("\e[48;5;%dm", DARK);
        }

        printf("\e[1m");
        (square & BLACK) ? printf("\e[38;5;232m") : printf("\e[37m");
        print_piece(square);
        printf("\e[0m");

        if (i % 8 == 7)
            printf("\u2551\e[3E\e[4C");
    }
    printf("\e[1E\n");
}

/* Prints the board, but from black's perspective. */
void print_flipped(Board* board)
{
    int i;
    printf("  \u2554");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u2557");
    for (i = 63; i >= 0; --i)
    {
        uint8_t square = board->position[i];
        if ((63 - i) % 8 == 0)
            printf("\n%d \u2551", ((63 - i) / 8) + 1);
        /* Bullshit that colors them checkered-like 
         * - Courtesy of Zach Gorman
         */
        if (!(!(i & 1) ^ !(i & 8))) 
            printf("\e[48;5;%dm", LIGHT);
        else
            printf("\e[48;5;%dm", DARK);
        printf(" \e[1m");
        if (square & PAWN)
            (square & BLACK) ? printf("\e[38;5;232mp") : printf("\e[37mP");
        else if (square & BISHOP)
            (square & BLACK) ? printf("\e[38;5;232mb") : printf("\e[37mB");
        else if (square & KNIGHT)
            (square & BLACK) ? printf("\e[38;5;232mn") : printf("\e[37mN");
        else if (square & ROOK)
            (square & BLACK) ? printf("\e[38;5;232mr") : printf("\e[37mR");
        else if (square & QUEEN)
            (square & BLACK) ? printf("\e[38;5;232mq") : printf("\e[37mQ");
        else if (square & KING)
            (square & BLACK) ? printf("\e[38;5;232mk") : printf("\e[37mK");
        else
            printf(" ");
        printf(" \e[0m");

        if ((63 - i) % 8 == 7)
            printf("\u2551");
        else
            printf("\u2502");
        if ((63 - i) % 8 == 7 && (63 - i) / 8 != 7)
        {
            printf("\n  \u2551\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c"
                    "\u2500\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500"
                    "\u2500\u2500\u253c\u2500\u2500\u2500\u253c\u2500\u2500"
                    "\u2500\u253c\u2500\u2500\u2500\u2551");
        }
    }
    printf("\n  ");
    printf("\u255a");
    for (i = 1; i < 32; ++i)
        printf("\u2550");
    printf("\u255d\n ");
    for (i = 0; i < 8; i++)
        printf("   %c", 'h' - i);
    printf("\n");
}

/* Converts a cstring to an int */
int string_to_int(char* str)
{
    int digits = strlen(str);
    int i;
    int num = 0;
    for (i = 0; i < digits; ++i)
    {
        int temp = str[i] - '0';
        int j;
        for (j = 0; j < digits - i - 1; ++j)
            temp *= 10;
        num += temp;
    }
    return num;
}

/* Loads a board state from a FEN */
void load_fen(Board* board, char* fen)
{
    empty_board(board);
    char* fen_copy = malloc(strlen(fen) + 1);
    strcpy(fen_copy, fen);
    char* token = NULL;
    token = strtok(fen_copy, " ");
    int square_ind = 0;
    int i = 0;
    char curr_char = token[i];
    while (curr_char)
    {
        if (curr_char == 'p')
            board->position[square_ind++] = PAWN   | BLACK;
        else if (curr_char == 'b')
            board->position[square_ind++] = BISHOP | BLACK;
        else if (curr_char == 'n')
            board->position[square_ind++] = KNIGHT | BLACK;
        else if (curr_char == 'r')
            board->position[square_ind++] = ROOK   | BLACK;
        else if (curr_char == 'q')
            board->position[square_ind++] = QUEEN  | BLACK;
        else if (curr_char == 'k')
        {
            board->bking_pos = square_ind;
            board->position[square_ind++] = KING   | BLACK;
        }
        else if (curr_char == 'P')
            board->position[square_ind++] = PAWN   | WHITE;
        else if (curr_char == 'B')   
            board->position[square_ind++] = BISHOP | WHITE;
        else if (curr_char == 'N')   
            board->position[square_ind++] = KNIGHT | WHITE;
        else if (curr_char == 'R')   
            board->position[square_ind++] = ROOK   | WHITE;
        else if (curr_char == 'Q')   
            board->position[square_ind++] = QUEEN  | WHITE;
        else if (curr_char == 'K')   
        {
            board->wking_pos = square_ind;
            board->position[square_ind++] = KING   | WHITE;
        }
        else if (curr_char <= '9' && curr_char >= '0')
            square_ind += curr_char - '0';
        i++;
        curr_char = token[i];
    }

    token = strtok(NULL, " ");
    if (token[0] == 'w')
        board->to_move = 0;
    else if (token[0] == 'b')
        board->to_move = 1;

    token = strtok(NULL, " ");
    i = 0;
    curr_char = token[i];
    while (curr_char)
    {
        if (curr_char == 'K')
            board->castling |= 0x08;
        else if (curr_char == 'Q')
            board->castling |= 0x04;
        else if (curr_char == 'k')
            board->castling |= 0x02;
        else if (curr_char == 'q')
            board->castling |= 0x01;
        else if (curr_char == '-')
            board->castling = 0x00;
        curr_char = token[++i];
    }

    token = strtok(NULL, " ");
    if (!strcmp(token, "-"))
        board->en_p = -1;
    else
        board->en_p = ((token[0] - 'a') + ('8' - token[1]) * 8);

    token = strtok(NULL, " ");
    board->halfmoves = string_to_int(token);

    token = strtok(NULL, " ");
    board->moves = string_to_int(token);

    free(fen_copy);
}

/* populates fen with the current board state in FEN format */
void export_fen(Board* board, char* fen)
{
    int str_ind = 0;
    int i;
    for (i = 0; i < 100; ++i)
        fen[i] = 0;
    int empty = 0;
    for (i = 0; i < 64; ++i)
    {
        uint8_t square = board->position[i];
        if (i % 8 == 0 && i)
        {
            if (empty)
            {
                fen[str_ind++] = '0' + empty;
                empty = 0;
            }
            fen[str_ind++] = '/';
        }
        if (square)
        {
            if (empty)
            {
                fen[str_ind++] = '0' + empty;
                empty = 0;
            }
            if (square & KNIGHT)
                fen[str_ind] = 'N';
            else if (square & PAWN)
                fen[str_ind] = 'P';
            else if (square & BISHOP)
                fen[str_ind] = 'B';
            else if (square & ROOK)
                fen[str_ind] = 'R';
            else if (square & QUEEN)
                fen[str_ind] = 'Q';
            else if (square & KING)
                fen[str_ind] = 'K';
            if (square & BLACK)
                fen[str_ind] += 32;
            str_ind++;
        }
        else
            empty++;
    }

    if (empty)
        fen[str_ind++] = '0' + empty;

    fen[str_ind++] = ' ';
    if (board->to_move)
        fen[str_ind++] = 'b';
    else
        fen[str_ind++] = 'w';
    fen[str_ind++] = ' ';
    if (board->castling & 0x08)
        fen[str_ind++] = 'K';
    if (board->castling & 0x04)
        fen[str_ind++] = 'Q';
    if (board->castling & 0x02)
        fen[str_ind++] = 'k';
    if (board->castling & 0x01)
        fen[str_ind++] = 'q';
    if (!board->castling)
        fen[str_ind++] = '-';
    fen[str_ind++] = ' ';
    if (board->en_p != -1)
    {
        fen[str_ind++] = board->en_p % 8 + 'a';
        fen[str_ind++] = 8 - board->en_p / 8 + '0';
    }
    else
        fen[str_ind++] = '-';
    fen[str_ind++] = ' ';
    str_ind += sprintf(fen + str_ind, "%d", board->halfmoves);
    fen[str_ind++] = ' ';
    sprintf(fen + str_ind, "%d", board->moves);
}

char* export_moves(Board* board)
{
    char* moves = malloc((int)board->history_count * 5 + 1);
    int str_ind = 0;
    int i;
    for (i = 0; i < board->history_count; ++i)
    {
        Move record = board->history[i];
        if (record.src_file != -1)
            str_ind += sprintf(moves + str_ind, "%c", 'a' + record.src_file);
        if (record.src_rank != -1)
            str_ind += sprintf(moves + str_ind, "%c", '0' + 8 - record.src_rank);
        if (record.dest != -1)
        {
            str_ind += sprintf(moves + str_ind, "%c", record.dest % 8 + 'a');
            str_ind += sprintf(moves + str_ind, "%c", 8 - record.dest / 8 + '0');
        }
        str_ind += sprintf(moves + str_ind, " ");
    }
    moves[str_ind] = '\0';
    return moves;
}

/* Returns a cstring on the heap which contains a PGN of the game */
char* export_pgn(Board* board)
{
    char* pgn = malloc(strlen(board->white_name) + strlen(board->black_name)
                + 10 * (int)board->history_count + 32);
    int str_ind = 0;
    str_ind += sprintf(pgn + str_ind, "[White \"%s\"]\n", board->white_name);
    str_ind += sprintf(pgn + str_ind, "[Black \"%s\"]\n", board->black_name);
    int i;
    for (i = 0; i < board->history_count; ++i)
    {
        Move record = board->history[i];
        if (i % 2 == 0)
            str_ind += sprintf(pgn + str_ind, "%d. ", i / 2 + 1);
        if (record.src_piece & KING)
            str_ind += sprintf(pgn + str_ind, "K");
        if (record.src_piece & QUEEN)
            str_ind += sprintf(pgn + str_ind, "Q");
        if (record.src_piece & ROOK)
            str_ind += sprintf(pgn + str_ind, "R");
        if (record.src_piece & KNIGHT)
            str_ind += sprintf(pgn + str_ind, "N");
        if (record.src_piece & BISHOP)
            str_ind += sprintf(pgn + str_ind, "B");
        if (record.src_file != -1)
            str_ind += sprintf(pgn + str_ind, "%c", 'a' + record.src_file);
        if (record.src_rank != -1)
            str_ind += sprintf(pgn + str_ind, "%c", '0' + 8 - record.src_rank);
        if (record.piece_taken)
            str_ind += sprintf(pgn + str_ind, "x");
        if (record.dest != -1)
        {
            str_ind += sprintf(pgn + str_ind, "%c", record.dest % 8 + 'a');
            str_ind += sprintf(pgn + str_ind, "%c", 8 - record.dest / 8 + '0');
        }
        if (record.promotion)
        {
            str_ind += sprintf(pgn + str_ind, "=");
            if (record.promotion & QUEEN)
                str_ind += sprintf(pgn + str_ind, "Q");
            if (record.promotion & ROOK)
                str_ind += sprintf(pgn + str_ind, "R");
            if (record.promotion & KNIGHT)
                str_ind += sprintf(pgn + str_ind, "N");
            if (record.promotion & BISHOP)
                str_ind += sprintf(pgn + str_ind, "B");
        }
        if (record.castle == 0)
            str_ind += sprintf(pgn + str_ind, "O-O");
        if (record.castle == 1)
            str_ind += sprintf(pgn + str_ind, "O-O-O");
        if (record.gave_check && !record.game_over)
            str_ind += sprintf(pgn + str_ind, "+");
        if (record.game_over & CHECKMATE)
        {
            str_ind += sprintf(pgn + str_ind, "#");
            if (board->to_move)
                str_ind += sprintf(pgn + str_ind, " 1-0");
            else
                str_ind += sprintf(pgn + str_ind, " 0-1");
        }
        if (record.game_over > CHECKMATE)
            str_ind += sprintf(pgn + str_ind, " 1/2-1/2");
        str_ind += sprintf(pgn + str_ind, " ");
    }
    pgn[str_ind] = '\0';
    return pgn;
}
