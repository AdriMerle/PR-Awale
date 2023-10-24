#ifndef AWALE_H
#define AWALE_H

#include "ustruct.h"

// Game actions
void copy(AwaleGame* source, AwaleGame* destination); // Copies the game
void save_game(const AwaleGame* game, const char* filename); // Saves the game
void load_game(AwaleGame* game, const char* filename); // Loads the game
int randomPlayer(); // Chooses the starting player
void init_game(AwaleGame* game); // initialize the game state
void get_board(const AwaleGame* game, char* board); // print the board to the console
int play_move(AwaleGame* game, int column); // play a move for the current player, return 1 if the game is over
int get_winner(AwaleGame* game); // get the winner of the game, or -1 if still playing or -2 if tie

#endif /* guard */
