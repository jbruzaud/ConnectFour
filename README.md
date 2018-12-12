# ConnectFour
A simple client-server Connect Four game over TCP (school project)

## Server

This server allow any number of players to connect to the server and choose one player to play with.
Players connect to the lobby where they can chat with each other or engage in a game. Scores are stored in a file, associated with a nickname.

### Installation
Simply compile the files :

    gcc GameLogic.c server.c -o server
    ./server

## Client

### Installation
Simply compile the files :

    gcc affichage.c InputManage.c main.c -o client
    ./client
    
