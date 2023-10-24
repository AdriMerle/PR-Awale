# PR-Awale
Une implémentation d'une application client/server du jeu Awale.

# Lancement de l'application
**Côté serveur :**
./server

**Côté client :**
./client [AdresseServeur] [NomD'Utilisateur]

# Liste de commandes :

| Commande | Action |
| -------- | ------ |
| /d | Affiche la liste des joueurs connectés
| /c [username] | Challenge le joueur username   
| /y | Accepte une invitation   
| /n | Refuse une invitation   
| /p [1-6] | Joue le coup [1-6]   
| /a [description] | Modifie votre description   
| /b [username] | Affiche la description du joueur 'username'   
| /m [username] [message] | Envoie un message privé à 'username'   
| /h | Affiche la liste des commandes   