#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "board.h"
#include "io.h"


char* to_lowercase(char* s)
{
    if (!s)
        return NULL;
    char* ns = malloc(strlen(s) + 1);
    int pos = 0;
    char c;
    while((c = s[pos]) != '\0')
    {
        if (c == '\n')
            break;
        if (c >= 'A' && c <= 'Z')
            c += 32;
        ns[pos] = c;
        pos++;
    }
    ns[pos] = '\0';
    return ns;
}

struct Command
{
    char* name;
    int (*func)(Board* board, int n_tokens, char tokens[][256]);
    int is_networked;
    char* help;
};

int StatusCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "status"))
        return -1;
    board_stats(board);
    return 0;
}

int ExitCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "exit"))
        return -1;
    return STOP;
}

int UndoCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "undo"))
        return -1;
    UndoMove(board);
    return 0;
}

int FenCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "fen"))
        return -1;
    char fen[FEN_SIZE];
    export_fen(board, fen);
    snprintf(board->notes, NOTES_LENGTH, "%s\n", fen);
    return 0;
}

int PGNCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "pgn"))
        return -1;
    char* pgn = export_pgn(board);
    snprintf(board->notes, NOTES_LENGTH, "%s\n", pgn);
    free(pgn);
    return 0;
}

int MovesCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "moves"))
        return -1;
    char* moves = export_moves(board);
    snprintf(board->notes, NOTES_LENGTH, "%s\n", moves);
    free(moves);
    return 0;
}

int FlipCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "flip"))
        return -1;
    return FLIPPED;
}

int AutoFlipCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "autoflip"))
        return -1;
    printf("am I flipping here?\n");
    return AUTOFLIP;
}

int NewCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "new"))
        return -1;
    default_board(board);
    return NEWGAME;
}

int GoCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "go"))
        return -1;
    return 0;
}

int SaveCommand(Board* board, int n_tokens, char tokens[][256])
{
    if (n_tokens != 1 || strcmp(tokens[0], "save"))
        return -1;
    char* pgn = export_pgn(board);
    printf("Enter Filename: ");
    char filename[50];
    scanf("%30s", filename);
    FILE* file = fopen(filename, "w");
    fwrite(pgn, sizeof(char), strlen(pgn), file);
    fwrite("\n", sizeof(char), 1, file);
    snprintf(board->notes, NOTES_LENGTH, "Saved to %s\n", filename);
    fclose(file);
    free(pgn);
    return 0;
}


struct Command commands[] = {
    {"status", StatusCommand, 0, "See debug stats about boardstate"},
    {"undo", UndoCommand, 1, "Undo last move"},
    {"fen", FenCommand, 0, "Print FEN of current board position to screen"},
    {"pgn", PGNCommand, 0, "Print PGN game to screen"},
    {"moves", MovesCommand, 0, "Print game's moves in long notation"},
    {"flip", FlipCommand, 0, "Flip board orientation"},
    {"autoflip", AutoFlipCommand, 0, "Flip board orientation on every turn"},
    {"new", NewCommand, 1, "Start a new game"},
    {"go", GoCommand, 1, "Start a game between two loaded engines"},
    {"save", SaveCommand, 0, "Save game PGN to file"},
    {"exit", ExitCommand, 1, "Exit Program"},
    { 0 }
};

int tokenize_command(char input[COMMAND_LENGTH], char tokens[][256])
{
    char* save_ptr;
    char* token;

    int i = 0;
    while (input[i] != '\0')
    {
        if (input[i] == '\n')
            input[i] = '\0';
        else
            i++;
    }

    int terms = 0;
    token = strtok_r(input, " ", &save_ptr);
    if (token == NULL)
        return -1;
    while (token != NULL)
    {
        char* lowercase_token = to_lowercase(token);
        memcpy(tokens[terms], lowercase_token, strlen(lowercase_token) + 1);
        free(lowercase_token);
        token = strtok_r(NULL, " ", &save_ptr);
        terms++;
    }
    return terms;
}

int is_networked_command(char input[COMMAND_LENGTH])
{
    int terms;
    char tokens[256][256];
    char input_copy[COMMAND_LENGTH];
    memcpy(input_copy, input, COMMAND_LENGTH);
    terms = tokenize_command(input_copy, tokens);

    if (!strcmp(tokens[0], "help"))
        return 0;
    int i = 0;
    while (commands[i].name != NULL)
    {
        if (!strcmp(commands[i].name, tokens[0]))
        {
            return commands[i].is_networked;
        }
        i++;
    }
    return 1;
}

int ProcessCommand(Board* board, char input[COMMAND_LENGTH])
{
    int terms;
    char tokens[256][256];
    terms = tokenize_command(input, tokens);
    if (terms < 0)
        return -1;

    int return_val = -2;
    int i;
    if (!strcmp(tokens[0], "help"))
    {
        i = 0;
        int chars_printed = 0;
        while (commands[i].name != NULL)
        {
            if (board->notes == NULL)
            {
                printf("%s - %s\n", commands[i].name, commands[i].help);
            }
            else
            {
              chars_printed += snprintf(
                      board->notes + chars_printed,
                      NOTES_LENGTH - chars_printed,
                      "%s - %s\n", commands[i].name, commands[i].help);
            }
            i++;
        }
        return 0;
    }
    else
    {
        i = 0;
        while (commands[i].name != NULL)
        {
            if (!strcmp(commands[i].name, tokens[0]))
            {
                return_val = commands[i].func(board, terms, tokens);
                if (return_val == -1)
                {
                    if (board->notes)
                    {
                      snprintf(board->notes, 256,
                               "Invalid usage of command %s\n",
                               commands[i].name);
                    }
                    else
                        printf("Invalid usage of command %s\n", commands[i].name);
                    return return_val;
                }
                return return_val;
            }
            ++i;
        }
    }
    return MOVE;
}
