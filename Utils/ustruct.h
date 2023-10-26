#define USERNAME_SIZE 20
#define BUF_SIZE    1024
#define MAX_OBSERVERS 10
typedef int SOCKET;

#include <stdio.h>
#include <stdlib.h>

// Game state
typedef struct {
	int board[2][6]; // the board, with 2 rows and 6 columns
	int player; // the current player, 0 or 1
	int score[2]; // the score of each player
   char playerA[USERNAME_SIZE];
   char playerB[USERNAME_SIZE];
} AwaleGame;

typedef struct Match {
	AwaleGame* game;
	struct Match* next;
   int en_cours;
   SOCKET sockObservers[MAX_OBSERVERS];
   int nb_observers;
} Match;

typedef struct Client
{
   SOCKET sock;
   char name[USERNAME_SIZE];
   struct Client* opponent;
   struct Match* match_en_cours; 
   int player_id;
   int score;
   char description[BUF_SIZE];
} Client;