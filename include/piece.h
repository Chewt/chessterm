#ifndef PIECE_H
#define PIECE_H

enum pieces
{
    pawn,
    rook,
    bishop,
    knight,
    king,
    queen
};

struct piece 
{
    int type;
    int rank;
    char file;
};

#endif
