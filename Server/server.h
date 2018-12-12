//
// Created by larnal on 29/12/17.
//

#ifndef PUISSANCE4SERVER_SERVER_H
#define PUISSANCE4SERVER_SERVER_H

#include "GameLogic.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PORT    2666
#define MAX_CLIENTS 	10
#define MAX_CLIENTS_JEU 2
#define MAX_GAMES 5

#define BUF_SIZE 1024



typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

typedef struct Client
{
    SOCKET sock;
    char name[BUF_SIZE];
    int id;
    int partie;
}Client;

typedef struct
{
    Client p1;
    Client p2;
    int partieId;
    int partieN;
    int gagnant;
    pid_t pid;
}data;


/* CONSTANTES */
const int demandeInput = 0;
const int afficherTexte = 1;
const int afficherJeu = 2;
const int finJeu = 3;

/* MESSAGES */
const char msgBienvenue[] = "Bonjour, la partie va commencer.\n";
const char msgTirageJoueur[] = "Tirer une pièce pour déterminer qui jouera en premier.\n";
const char msgValidationPremierJoueur[] = "%s joue en premier.\n"; // doit contenir %s nom du joueur
const char msgJoueurJoue[] = "Tour %d. C'est au tour de %s.\n"; // doit contenir %demandeInput tour et %s nom du joueur
const char msgDemandeColonne[] = "Choisissez la colonne où jouer votre pion.\n";
const char msgMauvaiseColonne[] = "Vous ne pouvez pas jouer ici. Choisissez une colonne non-pleine entre 1 et 7!\n";
const char msgFinPartie[] = "La partie est terminée! Veuillez attendre quelques instants...\n";
const char msgGagnant[] = "%s a gagné ! Félicitations !\n"; // doit contenir %s nom du joueur
const char msgEgalite[] = "Egalité !\n";
const char msgAccueil[] = "////////////////// PUISSANCE 4 F0R3V3R ////////////////////";
const char msgLobbyAccueil[] = "A : Actualiser la liste des joueurs.  Q : Se deconnecter  1 - %d : Choisir son adversaire.\n\nListe des joueurs : \n"; //%d = j
const char msgTropdeParties[] = "Il y a trop de parties en cours, merci d'attendre qu'une partie se termine.\n";
const char msgAuRevoir[] = "Merci de votre visite, au revoir !";
const char startgame[] = "DEBUTDUJEU";
const char msgConnexion[] = "vient de se connecter !\n";

/* METHODES */

data partie(data datajoueurs);

double tirage_au_sort();

void serialize_jeu(Jeux jeu, char *buffer);

int communication_clients(Client *liste, int typeMessage, const char *output, Jeux jeu, char *input);

int communication_client(SOCKET sock, int typeMessage, const char *output, Jeux jeu, char *input);

int rechercher_joueur(Client* clients, int id, int actual);

void message_lobby(Client c, Client *liste, int maxliste);

int update_lobby(Client *clients, Client **enAttente, int actual, fd_set *ptr);

static void remove_client(Client *clients, int to_remove, int *actual);

int read_client(SOCKET sock, char *buffer);

int write_clients(Client c, Client *clients, int max, char *buffer);

int nouvelle_connexion(Client c, Client *clients, int max);

int write_client(SOCKET sock, const char *buffer);

int init_connection(void);

void extinction(int sig);

int estFinStr(char i);



#endif /* guard */
