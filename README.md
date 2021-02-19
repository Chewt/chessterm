# What is it?
Chessterm is a terminal based chess api. 

It currently (almost) supports Standard Algebraic Notation for inputting the
moves. 

## Usage
A test implementation has been provided. Simply run  
`$make`  
to compile and start
the program with  
`$./chessterm [fen]`  
where fen is an optional string 
representing a chess position in the Forsyth-Edwards Notation (FEN) format.

example: `$ ./chessterm "rnbq1bnr/1ppp1ppp/6k1/4Q3/4P3/p7/PPP2PPP/RNBQKBNR w KQ - 2 8"`

If you do not provide a FEN, then the board will be initialized to a standard
chess board.

In the program you will be presented with a view of the current position and
a prompt. You can type  
`: exit`  
to exit the program,  
`: status`  
to view the current board information, or you can type a move in SAN notation
to make a move.
example: `: e4` or `: Nf3`  
Pieces must be uppercase, and files must be lowercase for the move to be read
correctly.

## Implementation Details
Squares on the board will be represented by 8 bit unsigned integers.
The leading bit will determine the color of the piece on the square

Empty square = 00000000 = 0  

Colors:    
| Color     | Binary    | Decimal|
| :---:     | :---:     | :---:  |
| white     | 00000000  | 0      |
| black     | 10000000  | 128    |

White pieces:  
| Color     | Binary    | Decimal|
| :---:     | :---:     | :---:  |
|pawn       | 00000001  | 1      |
|bishop     | 00000010  | 2      | 
|knight     | 00000100  | 4      |
|rook       | 00001000  | 8      |
|queen      | 00010000  | 16     | 
|king       | 00100000  | 32     | 

Black pieces:  
| Color     | Binary    | Decimal|
| :---:     | :---:     | :---:  |
|pawn       | 10000001  | 129    | 
|bishop     | 10000010  | 130    | 
|knight     | 10000100  | 132    |    
|rook       | 10001000  | 136    | 
|queen      | 10010000  | 144    | 
|king       | 10100000  | 160    | 

Bit-wise or a piece and a color together to get a colored piece.

The directions a piece can move are described below as relative positions in
the array that holds all of the pieces.

    -9|-8 |-7
    ----------
    -1| P |+1  
    ----------  
    +7|+8 |+9

