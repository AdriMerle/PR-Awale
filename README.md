# PR-Awale
Une implémentation d'une application client/server du jeu Awale.

Par Antonin S et Adrian M.

# Compilation
**Côté serveur :**
make server

**Côté client :**
make client

# Lancement de l'application
**Côté serveur :**
./server

**Côté client :**
./client [AdresseServeur] [username]

# Liste des commandes

| Commande | Action |
| --- | --- |
| /a [description] | Modifie votre description |
| /b [username] | Affiche la description du joueur 'username' |
| /c [username] | Challenge le joueur 'username' |
| /d | Affiche la liste des joueurs connectés |
| /g | Affiche la liste des parties en cours
| /h | Affiche la liste des commandes |
| /m [username] [message] | Envoie un message privé à 'username' |
| /n | Refuse une invitation |
| /o [username] | Observe la partie du joueur 'username' |
| /p [1-6] | Joue le coup [1-6] |
| /q [username] | Arrête d'observer la partie du joueur 'username' |
| /r | Affiche le classement des joueurs |
| /y | Accepte une invitation |
