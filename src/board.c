#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

void empty_board(struct board* board)
{
    board->to_move = 0;
    board->castling = 0x00;
    board->en_p = -1;
    board->halfmoves = 0;
    board->moves = 1;
    int i;
    for (i = 0; i < 64; ++i)
        board->position[i] = 0;
}

void default_board(struct board* board)
{
    int i;
    empty_board(board);
    board->castling = 0x0F;
    board->position[0] = rook   | black;
    board->position[1] = knight | black;
    board->position[2] = bishop | black;
    board->position[3] = queen  | black;
    board->position[4] = king   | black;
    board->position[5] = bishop | black;
    board->position[6] = knight | black;
    board->position[7] = rook   | black;
    for (i = 0; i < 8; ++i)
    {
        board->position[i + 8]     = pawn | black;
        board->position[i + 8 * 6] = pawn | white;
    }
    board->position[0 + 8 * 7] = rook   | white;
    board->position[1 + 8 * 7] = knight | white;
    board->position[2 + 8 * 7] = bishop | white;
    board->position[3 + 8 * 7] = queen  | white;
    board->position[4 + 8 * 7] = king   | white;
    board->position[5 + 8 * 7] = bishop | white;
    board->position[6 + 8 * 7] = knight | white;
    board->position[7 + 8 * 7] = rook   | white;
}

void board_stats(struct board* board)
{
    if (board->to_move == 1)
        printf("Black");
    else if (board->to_move == 0)
        printf("White");
    printf(" to move.\nCastling: ");
    printf("%#X ", board->castling);
    if (board->castling & 0x08)
        printf("K");
    if (board->castling & 0x04)
        printf("Q");
    if (board->castling & 0x02)
        printf("k");
    if (board->castling & 0x01)
        printf("q");
    printf("\nEn pessant: %d\n", board->en_p);
    printf("Halfmoves: %d\nFullmoves: %d\n", board->halfmoves, board->moves);
}

void print_board(struct board* board)
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
            printf("\e[7m");
        printf(" ");
        if (square & pawn)
            (square & black) ? printf("p") : printf("P");
        else if (square & bishop)
            (square & black) ? printf("b") : printf("B");
        else if (square & knight)
            (square & black) ? printf("n") : printf("N");
        else if (square & rook)
            (square & black) ? printf("r") : printf("R");
        else if (square & queen)
            (square & black) ? printf("q") : printf("Q");
        else if (square & king)
            (square & black) ? printf("k") : printf("K");
        else
            printf(" ");
        printf(" ");

        if (!(!(i & 1) ^ !(i & 8)))
            printf("\e[0m");

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

int string_to_int(char* str)
{
    int digits = strlen(str);
    int i;
    int num = 0;
    for (i = 0; i < digits; ++i)
    {
        int temp = str[i] - '0';
        int j;
        for (j = 0; j < digits - i - 1; ++i)
            temp *= 10;
        num += temp;
    }
    return num;
}

void load_fen(struct board* board, char* fen)
{
    empty_board(board);
    printf("%s\n", fen);
    printf("%#X\n", board->castling);
    char* fen_copy = malloc(strlen(fen) + 1);
    strcpy(fen_copy, fen);
    char* saveptr = NULL;
    char* token = NULL;
    token = strtok_r(fen_copy, " ", &saveptr);
    printf("%s\n", token);
    int square_ind = 0;
    int i = 0;
    char curr_char = token[i];
    while (curr_char)
    {
        printf("%c %#X %d\n", curr_char, board->castling, square_ind);
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
            board->position[square_ind++] = king   | black;
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
            board->position[square_ind++] = king   | white;
        else if (curr_char <= '9' && curr_char >= '0')
            square_ind += curr_char - '0';
        i++;
        curr_char = token[i];
    }
    printf("%#X\n", board->castling);

    token = strtok_r(NULL, " ", &saveptr);
    if (token[0] == 'w')
        board->to_move = 0;
    else if (token[0] == 'b')
        board->to_move = 1;

    token = strtok_r(NULL, " ", &saveptr);
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

    token = strtok_r(NULL, " ", &saveptr);
    if (!strcmp(token, "-"))
        board->en_p = -1;
    else
        board->en_p = ((token[0] - 'a') + ('8' - token[1]) * 8);

    token = strtok_r(NULL, " ", &saveptr);
    board->halfmoves = string_to_int(token);

    token = strtok_r(NULL, " ", &saveptr);
    board->moves = string_to_int(token);
    
    free(fen_copy);
}

void move_square(struct board* board, int dest, int src)
{
    board->position[dest] = board->position[src];
    board->position[src] = 0;
    board->moves++;
    board->to_move = !board->to_move;
}

void move_verbose(struct board* board, char* dest, char* src)
{
    uint8_t source_num = src[0] - 'a' + ('8' - src[1]) * 8;
    uint8_t dest_num = dest[0] - 'a' + ('8' - dest[1]) * 8;
    move_square(board, dest_num, source_num);
}

uint8_t find_attacker(struct board* board, int square, uint8_t piece)
{
    uint8_t found = 0xFF;
    if (board->position[square] && !((board->position[square] & 0x80) ^
                                     (board->to_move << 7)))
        return found;

    if (piece & knight)
    {
        if (board->position[square - 17] == piece)
            found = square - 17;
        else if (board->position[square - 10] == piece)
            found = square - 10;
        else if (board->position[square - 15] == piece)
            found = square - 15;
        else if (board->position[square - 6] == piece)
            found = square - 6;
        else if (board->position[square + 6] == piece)
            found = square + 6;
        else if (board->position[square + 15] == piece)
            found = square + 15;
        else if (board->position[square + 10] == piece)
            found = square + 10;
        else if (board->position[square + 17] == piece)
            found = square + 17;
    }
    if (piece & rook || piece & queen)
    {
        int i = square;
        while (i % 8 < 8)
        {
            i--;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
        i = square;
        while (i % 8 > 0)
        {
            i++;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
        i = square;
        while (i / 8 < 8)
        {
            i -= 8;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
        i = square;
        while (i / 8 > 0)
        {
            i += 8;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
    }
    if (piece & bishop || piece & queen)
    {
        int i = square;
        while (i % 8 < 8 && i / 8 < 8)
        {
            i -= 9;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
        i = square;
        while (i % 8 > 0 && i / 8 < 8)
        {
            i -= 7;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
        i = square;
        while (i % 8 < 8 && i / 8 > 0)
        {
            i += 7;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
        i = square;
        while (i % 8 > 0 && i / 8 > 0)
        {
            i += 9;
            if (board->position[i] != 0 && board->position[i] == piece)
                found = i;
            else if (board->position[i])
                break;
        }
    }
    if (piece & king || piece & pawn)
    {
        if (board->position[square - 1] == piece)
            found = square - 1;
        if (board->position[square + 1] == piece)
            found = square + 1;
        if (board->position[square - 9] == piece)
            found = square - 9;
        if (board->position[square + 9] == piece)
            found = square + 9;
        if (board->position[square - 7] == piece)
            found = square - 7;
        if (board->position[square + 7] == piece)
            found = square + 7;
        if (board->position[square - 8] == piece)
            found = square - 8;
        if (board->position[square + 8] == piece)
            found = square + 8;
    }
    return found;
}

void move_san(struct board* board, char* move)
{
    uint8_t sourcepiece = pawn;
    uint8_t promotionpiece = queen;
    uint8_t sourcerank = 0;
    uint8_t sourcefile = 0;
    uint8_t destrank = 0;
    uint8_t destfile = 0;
    int ind = 0;
    char curr_char = move[ind];
    if (curr_char == 'B')
        sourcepiece = bishop;
    else if (curr_char == 'N')
        sourcepiece = knight;
    else if (curr_char == 'R')
        sourcepiece = rook;
    else if (curr_char == 'Q')
        sourcepiece = queen;
    else if (curr_char == 'K')
        sourcepiece = king;
    else
        destfile = curr_char - 'a';
    if (board->to_move == 1)
        sourcepiece |= black;
    curr_char = move[++ind];
    while (curr_char)
    {
        if (curr_char <= 'h' && curr_char >= 'a')
        {
            if (destfile)
                sourcefile = destfile;
            destfile = curr_char - 'a';
        }
        else if (curr_char <= '9' && curr_char >= '0')
        {
            if (destrank)
                sourcerank = destrank;
            destrank = ('8' - curr_char) * 8;
        }
        curr_char = move[++ind];
    }
    uint8_t found = find_attacker(board, destrank + destfile, sourcepiece);
    if (found != 0xFF)
        move_square(board, destrank + destfile, found);
    else
        printf("You fool, thats not valid move!\n");
}
