#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "settings.h"

#ifndef SETTINGS
#define LIGHT 179
#define DARK  58
#endif

void board_stats(Board* board)
{
    if (board->to_move == 1)
        printf("Black");
    else if (board->to_move == 0)
        printf("White");
    printf(" to move.\nCastling: ");
    if (board->castling & 0x08)
        printf("K");
    if (board->castling & 0x04)
        printf("Q");
    if (board->castling & 0x02)
        printf("k");
    if (board->castling & 0x01)
        printf("q");
    printf("\nEn pessant: ");
    if (board->en_p == -1)
        printf("None\n");
    else
        printf("%c%d\n", (board->en_p % 8) + 'a', 8 - (board->en_p / 8));
    printf("Halfmoves: %d\nFullmoves: %d\n", board->halfmoves, board->moves);
    printf("Whiteking_pos: %c%d\n", (board->wking_pos % 8) + 'a',
            8 - (board->wking_pos / 8));
    printf("Blackking_pos: %c%d\n", (board->bking_pos % 8) + 'a',
            8 - (board->bking_pos / 8));
}

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
            board->position[square_ind++] = pawn   | black;
        else if (curr_char == 'b')
            board->position[square_ind++] = bishop | black;
        else if (curr_char == 'n')
            board->position[square_ind++] = knight | black;
        else if (curr_char == 'r')
            board->position[square_ind++] = rook   | black;
        else if (curr_char == 'q')
            board->position[square_ind++] = queen  | black;
        else if (curr_char == 'k')
        {
            board->bking_pos = square_ind;
            board->position[square_ind++] = king   | black;
        }
        else if (curr_char == 'P')
            board->position[square_ind++] = pawn   | white;
        else if (curr_char == 'B')   
            board->position[square_ind++] = bishop | white;
        else if (curr_char == 'N')   
            board->position[square_ind++] = knight | white;
        else if (curr_char == 'R')   
            board->position[square_ind++] = rook   | white;
        else if (curr_char == 'Q')   
            board->position[square_ind++] = queen  | white;
        else if (curr_char == 'K')   
        {
            board->wking_pos = square_ind;
            board->position[square_ind++] = king   | white;
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

char* export_fen(Board* board)
{
    char* fen = malloc(100);
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
            if (square & knight)
                fen[str_ind] = 'N';
            else if (square & pawn)
                fen[str_ind] = 'P';
            else if (square & bishop)
                fen[str_ind] = 'B';
            else if (square & rook)
                fen[str_ind] = 'R';
            else if (square & queen)
                fen[str_ind] = 'Q';
            else if (square & king)
                fen[str_ind] = 'K';
            if (square & black)
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
    return fen;
}

char* export_pgn(Board* board)
{
    char* pgn = malloc(10 * board->history_count + 10);
    int str_ind = 0;
    int i;
    for (i = 0; i < board->history_count; ++i)
    {
        Move record = board->history[i];
        if (i % 2 == 0)
            str_ind += sprintf(pgn + str_ind, "%d. ", i / 2 + 1);
        if (record.src_piece & king)
            str_ind += sprintf(pgn + str_ind, "K");
        if (record.src_piece & queen)
            str_ind += sprintf(pgn + str_ind, "Q");
        if (record.src_piece & rook)
            str_ind += sprintf(pgn + str_ind, "R");
        if (record.src_piece & knight)
            str_ind += sprintf(pgn + str_ind, "N");
        if (record.src_piece & bishop)
            str_ind += sprintf(pgn + str_ind, "B");
        if (record.src_file != -1)
            str_ind += sprintf(pgn + str_ind, "%c", 'a' + record.src_file);
        if (record.src_rank != -1)
            str_ind += sprintf(pgn + str_ind, "%c", '0' + record.src_rank);
        if (record.piece_taken)
            str_ind += sprintf(pgn + str_ind, "x");
        if (record.dest != -1)
        {
            str_ind += sprintf(pgn + str_ind, "%c", record.dest % 8 + 'a');
            str_ind += sprintf(pgn + str_ind, "%c", 8 - record.dest / 8 + '0');
        }
        if (record.castle == 0)
            str_ind += sprintf(pgn + str_ind, "O-O");
        if (record.castle == 1)
            str_ind += sprintf(pgn + str_ind, "O-O-O");
        if (record.gave_check && !record.game_over)
            str_ind += sprintf(pgn + str_ind, "+");
        if (record.game_over == 1)
            str_ind += sprintf(pgn + str_ind, "#");
        if (record.game_over == 2)
            str_ind += sprintf(pgn + str_ind, " 1/2-1/2");
        str_ind += sprintf(pgn + str_ind, " ");
    }
    pgn[str_ind] = '\0';
    return pgn;
}
