# What is it?
Chessterm is a terminal based chess api. 

It currently (almost) supports Standard Algebraic Notation for inputting the
moves. 

## Implementation Details
Squares on the board will be represented by 8 bit unsigned integers.
The leading bit will determine the color of the piece on the square

Empty square = 00000000 = 0

Colors:
white        = 00000000
black        = 10000000

White pieces:
pawn         = 00000001 = 1
bishop       = 00000010 = 2
knight       = 00000100 = 4
rook         = 00001000 = 8
queen        = 00010000 = 16
king         = 00100000 = 32

Black pieces:
pawn         = 10000001 = 129
bishop       = 10000010 = 130
knight       = 10000100 = 132
rook         = 10001000 = 136
queen        = 10010000 = 144
king         = 10100000 = 160

Bit-wise or a piece and a color together to get a colored piece.

The directions a piece can move are described below as relative positions in
the array that holds all of the pieces.

    -9|-8 |-7
    ----------
    -1| P |+1  
    ----------  
    +7|+8 |+9

