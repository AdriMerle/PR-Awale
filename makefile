client : Client/client2.c
	gcc -o client Client/client2.c

server : Serveur/server2.c Game/awale.c Utils/liste_chainee.c
	gcc -g -o server Serveur/server2.c Game/awale.c  Utils/liste_chainee.c

all : awale.c
	gcc -o awale awale.c
	./awale

trace : awale.c
	gcc -o awale awale.c -D TRACE
	./awale