# What is it?
Chessterm is a terminal based chess interface. 

![Imgur](https://imgur.com/Nl37KYV.png)

It was originally created to provide a bridge between several chess engines made
by a group of friends to compete with each other.

The current functionality includes:
 - Printing the current position of the board
 - Moving pieces on the board
 - Keeping track of whose turn it is
 - Validating any inputted moves
 - exporting FEN and PGN of the game
 - Detection of checkmate and stalemate
## Roadmap
Features that still need to be added
 - ~~Add UCI compatability~~ Minimally implemented
 - ~~Detect Stalemate~~
 - ~~Detect 3-fold repetition stalemate~~
 - Load/save FEN from/to files
 - ~~export~~/import PGN format
 - ~~Flip view of the board between black and white perspective~~
 - ~~Networking~~
## Usage
run
`$make`
to compile and start the program with
`$./chessterm`.   
A list of command line arguments can be seen with `$./chessterm --help`  

In the program you will be presented with a view of the current position and
a prompt. You can type  
`: help`  
to see all of the commands you can run while the program is runinng.
You can also type a move in SAN notation to make a move.  
Example: `: e4` or `: Nf3`  
Pieces must be uppercase, and files must be lowercase for the move to be read
correctly.

## Implementation Details
Squares on the board will be represented by an array of 64 8-bit unsigned 
integers.  The leading bit will determine the color of the piece on the square.

Empty square = 0  

Colors:    
| Color     | Binary    | Decimal|
| :---:     | :---:     | :---:  |
| white     | 00000000  | 0      |
| black     | 10000000  | 128    |

White pieces:  
| Piece     | Binary    | Decimal|
| :---:     | :---:     | :---:  |
|pawn       | 00000001  | 1      |
|bishop     | 00000010  | 2      | 
|knight     | 00000100  | 4      |
|rook       | 00001000  | 8      |
|queen      | 00010000  | 16     | 
|king       | 00100000  | 32     | 

Black pieces:  
| Piece     | Binary    | Decimal|
| :---:     | :---:     | :---:  |
|pawn       | 10000001  | 129    | 
|bishop     | 10000010  | 130    | 
|knight     | 10000100  | 132    |    
|rook       | 10001000  | 136    | 
|queen      | 10010000  | 144    | 
|king       | 10100000  | 160    | 

Bit-wise OR a piece and a color together to get a colored piece.

The directions a piece can move are described below as relative positions in
the array that holds all of the pieces.

    -9|-8 |-7
    ----------
    -1| P |+1  
    ----------  
    +7|+8 |+9

