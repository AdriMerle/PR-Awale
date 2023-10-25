#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"

Match* head;

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         int nb_char;
         if((nb_char = read_client(csock, buffer)) == -1 || !is_username_unique(buffer, actual, clients))
         {
            /* disconnected */
            send_usernames_to_client(clients, csock, actual);
            closesocket(csock);
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c = { csock };
         strncpy(c.name, buffer, USERNAME_SIZE - 1);
         c.name[nb_char] = '\0';
         clients[actual] = c;
         actual++;
         printf("Server : [%s] s'est connecté.\n", c.name);
         write_client(csock, "[Server] Vous êtes connecté !\n");
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client* client = &clients[i];
               int c = read_client(clients[i].sock, buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client->name, USERNAME_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, &client, actual, buffer, 1);
               }
               else
               {
                  analyze_message(clients, client, actual, buffer, c);
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client* sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender->sock != clients[i].sock)
      {
         if(from_server == 0)
         {
            strncpy(message, sender->name, USERNAME_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}

static void send_usernames_to_client(Client *clients, SOCKET sock, int actual)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   strncat(message, "Voici la liste des noms d'utilisateur déjà pris :", sizeof message - strlen(message) - 1);
   for(int i=0; i<actual; i++) {
      strncat(message, " | ", sizeof message - strlen(message) - 1);
      strncat(message, clients[i].name, sizeof message - strlen(message) - 1);
   }
   write_client(sock, message);
}


static int is_username_unique(char* username, int actual, Client * clients)
{
    for(int i = 0; i < actual; i++)
    {
        if(strcmp(clients[i].name, username) == 0)
        {
            /* username already exists */
            return 0;
        }
    }

    /* username is unique */
    return 1;
}

static Client* find_client_by_name(Client* clients, int actual, char* username) {
   for (int i = 0; i < actual; i++) {
      if (strcmp(clients[i].name, username) == 0) {
#ifdef TRACE
         printf("find_client_by_name resultat : p%sp\n", clients[i].name);
#endif         
         return &clients[i];
      }
   }
   // If no client with the given username is found, return a null pointer
   return NULL;
}

static void challenge(Client* clients, Client* client, int actual, char* buffer, int nb_char){
   char username[USERNAME_SIZE];
   strncpy(username, &buffer[3], nb_char);
#ifdef TRACE
   printf("Nombre de charactère dans la commande /c : %d \n", nb_char);
#endif
   username[nb_char - 3] = '\0';
   Client* opponent = find_client_by_name(clients, actual, username);
   if(opponent==NULL) {
      write_client(client->sock, "Aucun joueur avec ce nom :/");
      return;
   }else if (opponent->opponent != NULL){
      write_client(client->sock, "Ce joueur est déjà en train de jouer :(");
      return;
   }
   char message[BUF_SIZE] = "Le joueur ";
   strncat(message, client->name, sizeof message - strlen(message) - 1);
   strncat(message, " vous défie ! Acceptez-vous (y/n) ?", sizeof message - strlen(message) - 1);
   write_client(opponent->sock, message);
   opponent->opponent = client;
   client->opponent = opponent;
}

static void analyze_message(Client* clients, Client* client, int actual, char* buffer, int nb_char) {
   char username[USERNAME_SIZE];
   Client* client2;
   if(buffer[0]=='/') {
      printf("%s | %s\n", client->name,  buffer);
      switch (buffer[1]) {
         case 'a': // Modifie la description du client
            strncpy(client->description, &buffer[3], nb_char);
            client->description[nb_char - 3] = '\0';
            write_client(client->sock, "Description modifiée !");
            break;
         case 'b': // renvoie la description d'un client
            strncpy(username, &buffer[3], nb_char);
            username[nb_char - 3] = '\0';
            client2 = find_client_by_name(clients, actual, username);
            if(client2==NULL) {
               write_client(client->sock, "Aucun joueur avec ce nom :/");
            } else if (strcmp(client2->description, "")==0) {
               write_client(client->sock, "Ce joueur n'a pas de description :(");
            } else {
               write_client(client->sock, "La description est : ");
               write_client(client->sock, client2->description);
            }
            break;
         case 'c': // Challenge another player
#ifdef TRACE
            printf("Challenge started");
#endif
            if (client->opponent != NULL){
               write_client(client->sock, "Tu as déjà une invitation en cours :(");
            }else {
               challenge(clients, client, actual, buffer, nb_char);
            }
            break;
         case 'd': // Display all players name
#ifdef TRACE
            printf("Get username list");
#endif
            send_usernames_to_client(clients, client->sock, actual);
            break;
         case 'g' : //List all the current games that are played
            Match* p = head;
            if (p == NULL) {
               write_client(client->sock, "Aucun jeu en cours.");
            } else {
               while (p != NULL) {
                  write_client(client->sock, "Une partie est en cours entre ");
                  // write_client(client->sock, p->playerA->name);
                  // write_client(client->sock, " et ");
                  // write_client(client->sock, p->game->playerB->name);
                  // write_client(client->sock, ".\n");
                  // display_match_player_observer(p->game, client);
                  write_client(client->sock, "=================================\n");
                  p = p->next;
               }
            }
            break;
         case 'h': // Display help
            write_client(client->sock, "Liste des commandes :");
            write_client(client->sock, "\r\n/d : Affiche la liste des joueurs connectés");
            write_client(client->sock, "\r\n/c [username] : Challenge le joueur username");
            write_client(client->sock, "\r\n/y : Accepte une invitation");
            write_client(client->sock, "\r\n/n : Refuse une invitation");
            write_client(client->sock, "\r\n/p [1-6] : Joue le coup [1-6]");
            write_client(client->sock, "\r\n/a [description] : Modifie votre description");
            write_client(client->sock, "\r\n/b [username] : Affiche la description du joueur 'username'");
            write_client(client->sock, "\r\n/m [username] [message] : Envoie un message privé à 'username'");
            write_client(client->sock, "\r\n/h : Affiche la liste des commandes");
            break;
         case 'm' : // Send private message
            
            int i = 3; // Get starting index of private message
            while(buffer[i]!=' ') {
               i++;
            }
            strncpy(username, &buffer[3], i-3);
            username[i-3] = '\0';
            client2 = find_client_by_name(clients, actual, username);
            if(client2==NULL) {
               write_client(client->sock, "Aucun joueur avec ce nom :/");
            } else {
               char message[BUF_SIZE] = "[";
               strncat(message, client->name, sizeof message - strlen(message) - 1);
               strncat(message, "] : ", sizeof message - strlen(message) - 1);
               strncat(message, &buffer[i+1], sizeof message - strlen(message) - 1);
               write_client(client2->sock, message);
            }
            break;
         case 'n':
            if(client->opponent != NULL){
               write_client(client->opponent->sock, client->name);
               write_client(client->opponent->sock," a refusé ou annulé l'invitation :(");
               client->opponent->opponent = NULL;
               client->opponent = NULL;
            }else {
               write_client(client->sock, "Tu n'as aucune invitation en cours :(");
            }
            break;
         case 'p':
            if (client->match_en_cours==NULL){
               write_client(client->sock, "Tu n'as aucune partie en cours :(");
               break;
            }else if (client->match_en_cours->game->player!=client->player_id){
               write_client(client->sock, "Ce n'est pas à toi de jouer !");
               break;
            }else{
               int code = play_move(client->match_en_cours->game, atoi(&buffer[3])-1);
   #ifdef TRACE
               printf("play code = %d\n", code);
   #endif
               if (code==0){
                  write_client(client->sock, "Coup invalide. L'opposant serait affamé");
               } else if (code==-1){
                  write_client(client->sock, "Coup invalide. Donnez un chiffre entre 1 et 6.");
               } else {
                  display_match_player(client);
               }
            }
            break;
         case 'y': // Accept challenge
            if (client->opponent != NULL){
               printf("Une partie est lancée entre %s et %s !\n", client->name, client->opponent->name);
               Match* match = malloc(sizeof(Match));
               client->match_en_cours = match;
               client->player_id = 0;
               client->opponent->player_id = 1;
               client->opponent->match_en_cours = match;
               match->game = malloc(sizeof(AwaleGame));
               add_head(head, match);
               init_game(match->game, client->name, client->opponent->name);
               display_match_player(client);
            }else {
               write_client(client->sock, "Tu n'as aucune invitation en cours :(");
            }

            break;
         default:
            break;
      }
   } else {
      printf("Server : Message to all from %s\n", client->name);
      send_message_to_all_clients(clients, client, actual, buffer, 0);
   }
   memset(buffer, 0, sizeof(buffer));
}
static void display_match_player(Client* client){
   char board[BUF_SIZE];
   memset(board, 0, sizeof(board));
   get_board(client->match_en_cours->game, board);
   write_client(client->sock, board);
   write_client(client->opponent->sock, board);
   if(client->match_en_cours->game->player==client->player_id){
      write_client(client->sock, "A toi de jouer !");
      write_client(client->opponent->sock, "En attente du coup de l'adversaire.");
   } else {
      write_client(client->opponent->sock, "A toi de jouer !");
      write_client(client->sock, "En attente du coup de l'adversaire.");   
   }
}

//static void display_match_player_observer(AwaleGame* game, Client* client){
//   char board[BUF_SIZE];
//   memset(board, 0, sizeof(board));
//   get_board(game, board);
//   write_client(client->sock, board);
//}