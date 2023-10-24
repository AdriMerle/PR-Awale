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
         printf("Server : %s s'est connecté.\n", c.name);
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
   printf("%d\n", actual);
   Client* opponent = find_client_by_name(clients, actual, username);
   if(opponent==NULL) {
      write_client(client->sock, "Aucun joueur avec ce nom :/");
      return;
   }
   char message[BUF_SIZE] = "Le joueur ";
   strncat(message, client->name, sizeof message - strlen(message) - 1);
   strncat(message, " vous défie ! Acceptez-vous (y/n) ?", sizeof message - strlen(message) - 1);
   write_client(opponent->sock, message);
   opponent->opponent = client;
   client->opponent = opponent;
   printf("Opponent->opponent : %s.%s\n", opponent->name, opponent->opponent->name);
}

static void analyze_message(Client* clients, Client* client, int actual, char* buffer, int nb_char) {
   if(buffer[0]=='/') {
      printf("Command %s\n", buffer);
      switch (buffer[1]) {
         case 'd': // Display all players name
#ifdef TRACE
            printf("Get username list");
#endif
            send_usernames_to_client(clients, client->sock, actual);
            break;
         case 'c': // Challenge another player
            printf("Challenge started");
#ifdef TRACE
            printf("Challenge started");
#endif
            challenge(clients, client, actual, buffer, nb_char);
            break;
         case 'y': // Accepter une invitation
            printf("Une partie est lancée entre %s et %s !\n", client->name, client->opponent->name);
            Match* match = malloc(sizeof(Match));
            client->match_en_cours = match;
            client->player_id = 0;
            client->opponent->player_id = 1;
            client->opponent->match_en_cours = match;
            match->game = malloc(sizeof(AwaleGame));
            add_head(head, match);
            init_game(match->game);
            display_match(client);
            break;
         case 'n':
            write_client(client->opponent->sock, "Le joueur a décliné votre invitation :(");
            // TO DO : secure si refus invitation ou si acceptation alors qu'aucune invitation
            break;

         case 'p':
            int code = play_move(client->match_en_cours->game, atoi(&buffer[3])-1);
#ifdef TRACE
            printf("play code = %d\n", code);
#endif
            if (code==0){
               write_client(client->sock, "Coup invalide. L'opposant serait affamé");
            } else if (code==-1){
               write_client(client->sock, "Coup invalide. Donnez un chiffre entre 1 et 6.");
            } else {
               display_match(client);
            }
            break;
         case 'a': // Modifie la description du client
            strncpy(client->description, &buffer[3], nb_char);
            client->description[nb_char - 3] = '\0';
            write_client(client->sock, "Description modifiée !");
            break;
         case 'b': // renvoie la description d'un client
            char username[USERNAME_SIZE];
            strncpy(username, &buffer[3], nb_char);
            username[nb_char - 3] = '\0';
            Client* opponent = find_client_by_name(clients, actual, username);
            if(opponent==NULL) {
               write_client(client->sock, "Aucun joueur avec ce nom :/");
            } else {
               write_client(client->sock, "La description est : ");
               write_client(client->sock, opponent->description);
            }
            break;
         case 'h': // Help
            // TO DO
            break;
         default:
            break;
      }
   } else {
      printf("Message to all\n");
      send_message_to_all_clients(clients, client, actual, buffer, 0);
   }
   memset(buffer, 0, sizeof(buffer));
}
static void display_match(Client* client){
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