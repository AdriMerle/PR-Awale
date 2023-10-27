client : Client/client2.c
	gcc -o client Client/client2.c

server : Serveur/server2.c Game/awale.c Utils/liste_chainee.c
	gcc -o server Serveur/server2.c Game/awale.c  Utils/liste_chainee.c