# What is it?
## Play Chess in the terminal!

## Features
The current functionality includes:
 + Play against chess engines
 + Play against another user over the network
 + Load a game position from a FEN
 + Export games to PGN

Here is an example of how it looks in the terminal 
![Imgur](https://imgur.com/Nl37KYV.png)

## Roadmap
Features that still need to be added
 - ~~Add UCI compatability~~ Minimally implemented
 - ~~Detect Stalemate~~
 - ~~Detect 3-fold repetition stalemate~~
 - Load/save FEN from/to files
 - ~~export~~/import PGN format
 - ~~Flip view of the board between black and white perspective~~
 - ~~Networking~~

## Install
run `$make install` to compile the program and install it.

# Usage
A list of command line arguments can be seen with `$./chessterm --help`  
In the program you will be presented with a view of the current position and
a prompt. You can type  
`: help`  
to see all of the commands you can run while the program is runinng.
You can also type a move in SAN notation to make a move.  
Example: `: e4` or `: Nf3`  
Pieces must be uppercase, and files must be lowercase for the move to be read
correctly.

# Networking
To play against someone on the network, simple have on player run chessterm with
the command `chessterm -h` to act as the host, and the other player use the
command `chessterm -c IPADDRESS` to connect to the host. You can also use the
machine's hostname instead of the IP address if your network supports hostname
resolution.

# Engines
Any chess engine that supports the Universal Chess Interface (UCI) protocol can
be connected to chessterm with the `chessterm --white_engine="PATH/TO/ENGINE"`.
You can replace `--white_engine` with `--black_engine` to make the engine play
as black.
