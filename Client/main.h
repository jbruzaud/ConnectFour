//
// Created by larnal on 01/01/18.
//

#include "affichage.h"

#ifndef CLIENTTEST_MAIN_H
#define CLIENTTEST_MAIN_H

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PORT	 2666

#define BUF_SIZE 1024

#define demandeInput 0
#define afficherTexte 1
#define afficherJeu 2
#define finJeu 3


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;


int init_connection(const char *adresse);
void end_connection(int sock);
int lecture_serveur(SOCKET sock, int *boucle);
void write_server(SOCKET sock, const char *buffer);
static int read_server(SOCKET sock, char *buffer);

const char startgame[] = "DEBUTDUJEU";



int main();

#endif //CLIENTTEST_MAIN_H
