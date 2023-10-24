#include "awale.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// For test purposes
// int main() {
// 	AwaleGame game;
// 	init_game(&game);
// 	char board[1024];
// 	//load_game(&game, "save.sav");
// 	while (get_winner(&game) == -1) {
// 		printf("Player = %d\n", game.player);
// 		get_board(&game, board);
// 		int column;
// 		printf("Player %c, choose a column to play: ", 'A' + game.player);
// 		scanf("%d", &column);
// 		play_move(&game, column - 1);
// 		save_game(&game, "save.sav");
// 	}
// 	int winner = get_winner(&game);
// 	get_board(&game, board);
// 	if (winner == -2) {
// 		printf("It's a tie!\n");
// 	} else {
// 		printf("Player %c wins!\n", 'A' + winner);
// 	}
// 	return 0;
// }

void init_game(AwaleGame* game) {
	// Set the board to its initial configuration
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++) {
			game->board[i][j] = 4;
		}
	}
	// Set the current player to 0
	game->player = randomPlayer();
	// Set the score to 0
	game->score[0] = 0;
	game->score[1] = 0;
}

void get_board(const AwaleGame* game, char* board) {
	strcat(board,"\n    1 2 3 4 5 6\n");
	strcat(board,"  +-------------+\n");
	strcat(board,"A |");
	char buffer[1024];
	for (int j = 5; j >= 0; j--) {
		strcat(board," ");
		sprintf(&buffer, "%d", game->board[0][j]);
		strcat(board, &buffer);
	}
	strcat(board," |\nB |");
	for (int j = 0; j < 6; j++) {
		strcat(board," ");
		sprintf(&buffer, "%d", game->board[1][j]);
		strcat(board, &buffer);
	}
	strcat(board," |\n");

	strcat(board,"  +-------------+\n");
	strcat(board, "|score| A :");
	sprintf(&buffer, "%d", game->score[0]);
	strcat(board, &buffer);
	strcat(board, " | B : ");
	sprintf(&buffer, "%d", game->score[1]);
	strcat(board, &buffer);
	strcat(board,"\n");
}

int play_move(AwaleGame* game, int column) {
	// Invert column if player A
	if(game->player==0) {
		column = 5 - column;
	}

	// Check if the column is valid
	if (column < 0 || column > 5 || game->board[game->player][column] == 0) {
		printf("Invalid move!\n");
		return -1;
	}

	// Make a copy of the game
	AwaleGame game_copy;
	copy(game, &game_copy);

	// Play the move
	int initial_column = column;
	int initial_row = game->player;
	int seeds = game->board[game->player][column];
	game->board[game->player][column] = 0;
	int row = game->player;
	while (seeds > 0) {
		column = (column + 1) % 6;
		if (column == 0)
			row = 1 - row;
#ifdef TRACE
		printf("col=%d, row=%d, seeds=%d\n", column, row, seeds);
		printf("cond=%d\n", row != initial_row && column != initial_column);
#endif
		if (row == initial_row && column == initial_column)
			continue;
		else {
			game->board[row][column]++;
			seeds--;
		}
	}

    // Check if the last hole is on the opponent's side and has 2 or 3 seeds
    while(row != game->player && column >=0 && game->board[row][column] >= 2 && game->board[row][column] <= 3) {
        int captured_seeds = game->board[row][column];
        game->score[game->player] += captured_seeds;
        game->board[row][column] = 0;
		column--;
    }

	// Check if the opponent is not starving
	int sum=0;
	for (int i = 0; i < 6; i++) {
		sum += game->board[1-game->player][i];
	}
	if(sum==0) {
		copy(&game_copy, game);
		printf("Invalid move, the opponent would starve !\n");
		return 0;
	}

	// Switch player
	game->player = 1 - game->player;

	return 1;
}

int get_winner(AwaleGame* game) {
	// Check if the game is over
	int sumA = 0;
	int sumB = 0;
	for (int i = 0; i < 6; i++) {
		sumA += game->board[0][i];
		sumB += game->board[1][i];
	}
	if (sumA > 0 && sumB > 0) {
		return -1;
	} 
	else {
		game->score[0] += sumB;
		game->score[1] += sumA;
	}

	// Compute the winner
	if (game->score[0] > game->score[1]) {
		return 0;
	} else if (game->score[1] > game->score[0]) {
		return 1;
	} else { // tie
		return -2;
	}
}

int randomPlayer() {
	// Seed the random number generator with the current time
    srand(time(NULL));
    // Generate a random integer between 0 and 1
    return rand() % 2;
}


void save_game(const AwaleGame* game, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error: could not open file %s for writing\n", filename);
        return;
    }
    fwrite(game, sizeof(AwaleGame), 1, file);
    fclose(file);
}

void load_game(AwaleGame* game, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: could not open file %s for reading\n", filename);
        return;
    }
    fread(game, sizeof(AwaleGame), 1, file);
    fclose(file);
}

void copy(AwaleGame* source, AwaleGame* destination) {
    // Copy the board
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            destination->board[i][j] = source->board[i][j];
        }
    }

    // Copy the score
    for (int i = 0; i < 2; i++) {
        destination->score[i] = source->score[i];
    }

    // Copy the player
    destination->player = source->player;
}