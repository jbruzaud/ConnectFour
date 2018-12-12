//
// Created by larnal on 29/12/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/wait.h>


#include "server.h"

data partie(data datajoueurs)
{
    char buffer[BUF_SIZE];
    int joueurCourant; // désigne quel joueur joue ce tour
    Jeux jeu = {0}; // plateau du jeu puissance 4
    initJeu(jeu); // remplissage vide du plateau
    coord pion = {0, 0}; // coordonnées du pion joué
    int tour = 1; //Tour de jeu
    data dataend = datajoueurs;
    dataend.partieId = 666;

    Client joueurs[2] = {datajoueurs.p1, datajoueurs.p2}; // Récupération des données nécessaires au jeu

    printf("Début Partie n%d.\n", datajoueurs.partieId);

    for (int i = 0; i < MAX_CLIENTS_JEU; i++)
    {
        write_client(joueurs[i].sock, startgame); // MODE BLOQUANT d'input pour le client
    }

    /*------------------------Message d'accueil aux joueurs --------------------------*/
    if (communication_clients(joueurs, afficherTexte, msgBienvenue, NULL, NULL) == -1) return dataend;
    if (communication_clients(joueurs, afficherTexte, msgTirageJoueur, NULL, NULL) == -1) return dataend;

    /*---------------- tirage au sort du premier joueur -----------------------*/
    if (tirage_au_sort() > 0.5) joueurCourant = 0;
    else joueurCourant = 1;
    sprintf(buffer, msgValidationPremierJoueur, joueurs[joueurCourant].name);
    if (communication_clients(joueurs, afficherTexte, buffer, NULL, NULL) == -1) return dataend;

    /*--------------------- Boucle du jeu----------------------------*/
    while (!pionGagnant(jeu, pion, joueurCourant) && !plateauPlein(jeu)) // Tant que le jeu n'est pas plein ou gagné
    {

        if (tour != 1) joueurCourant ^= 1;//XOR pour osciller entre 0 et 1 sauf au 1er tour

        // ----------- AFFICHAGE PLATEAUX TOUS LES JOUEURS -----------------
        if (communication_clients(joueurs, afficherJeu, NULL, jeu, NULL) == -1) return dataend;
        //Affiche le message de début du tour
        sprintf(buffer, msgJoueurJoue, tour, joueurs[joueurCourant].name);
        if (communication_clients(joueurs, afficherTexte, buffer, NULL, NULL) == -1) return dataend;

        // ----------- JOUEUR COURANT JOUE PION -----------------
        // ------------------------------------------------------
        // Boucle pour demander la COLONNE où jouer le pion
        // si la COLONNE donnée n'est pas valide (hors jeu ou pleine), on reste dans la boucle
        int pionOK = 0;
        int posx;

        do
        {
            //Demande d'input colonne
            if (communication_client(joueurs[joueurCourant].sock, demandeInput, msgDemandeColonne, NULL,
                                     buffer) == -1)
                return dataend;
            posx = (int) (strtol(&buffer[0], NULL, 10) - 1); //Input en int

            if ((posx <= 6 && posx >= 0) && !colonnePleine(jeu, posx))
            {
                pion.x = posx;
                pionOK = 1; //validation du pion joué
            } else
            {
                if (communication_client(joueurs[joueurCourant].sock, afficherTexte, msgMauvaiseColonne, NULL, NULL) ==
                    -1)
                    return dataend;
            }
        } while (!pionOK);

        pion.y = jouerPion(jeu, pion.x, joueurCourant); // Le pion est joué et on récupère sa LIGNE
        tour++; // On incrémente le compteur de tour
    }

    /*--------------------------- FIN DE PARTIE (PLATEAU PLEIN OU VAINQUEUR -----------------*/
    if (communication_clients(joueurs, afficherJeu, NULL, jeu, NULL) == -1) return dataend; // Affichage plateau

    if (pionGagnant(jeu, pion, joueurCourant)) // Si joueur courant est gagnant
    {
        sprintf(buffer, msgGagnant, joueurs[joueurCourant].name);
        if (communication_clients(joueurs, afficherTexte, buffer, NULL, NULL) == -1)
            return dataend; //Affiche message victorieux
        datajoueurs.gagnant = joueurCourant; //Inscrit joueur gagnant dans data

    } else if (plateauPlein(jeu) && !pionGagnant(jeu, pion, joueurCourant))  // Si égalité (plateau plein)
    {
        if (communication_clients(joueurs, afficherTexte, msgEgalite, NULL, NULL) == -1)
            return dataend; // Affiche égalité
        datajoueurs.gagnant = -1; // Pas de gagnant (-1)
    }
    datajoueurs.pid = getpid(); // Inscription du pid du fils dans data (pour fermeture du bon fils par le père

    if (communication_clients(joueurs, finJeu, NULL, NULL, NULL) == -1)
        return dataend; //Envoi de FIN aux joueurs => MODE DYNAMIQUE d'input du client

    return datajoueurs;
}


double tirage_au_sort()
{
    srandom((unsigned int) time(NULL));
    return (random() / (double) RAND_MAX);
}

void serialize_jeu(Jeux jeu, char *buffer)
{
    int y = 0;
    int x = 0;
    for (y = 0; y < NLINE; y++)
    {
        for (x = 0; x < NCOL; x++)
        {
            buffer[y * NCOL + x] = (char) (jeu[x][y] + '0');
        }
    }
    buffer[y * NCOL + x] = '\0';
}

int communication_clients(Client *liste, int typeMessage, const char *output, Jeux jeu, char *input)
{
    for (int i = 0; i < MAX_CLIENTS_JEU; i++)
    {
        if (communication_client(liste[i].sock, typeMessage, output, jeu, input) == -1) return -1;
    }
    return 1;
}

int communication_client(SOCKET sock, int typeMessage, const char *output, Jeux jeu, char *input)
{
    char buffer[BUF_SIZE];
    char concat[BUF_SIZE - 1];
    int n;
    buffer[0] = (char) (typeMessage + '0');
    buffer[1] = '\0';
    if (typeMessage == afficherJeu)
    {
        serialize_jeu(jeu, concat);
        strcat(buffer, concat);
    } else if (typeMessage == finJeu) strcat(buffer, msgFinPartie);
    else strcat(buffer, output);
    n = write_client(sock, buffer);


    if (typeMessage == demandeInput && n != -1) return read_client(sock, input);
    return n;
}


int rechercher_joueur(Client *clients, int id, int actual)
{
    for (int i = 0; i <= actual; i++)
    {
        if (clients[i].id == id) return i;
    }
    return -1;
}

void message_lobby(Client c, Client *liste, int maxliste)
{
    char buffer[BUF_SIZE];
    char temp[BUF_SIZE];
    sprintf(buffer, msgLobbyAccueil, maxliste);
    for (int i = 0; i < maxliste; i++)
    {
        if (strcmp(c.name, liste[i].name) == 0) sprintf(temp, "%d - /// %s ///\n", i + 1, liste[i].name);
        else sprintf(temp, "%d - %s\n", i + 1, liste[i].name);
        strcat(buffer, temp);
    }
    write_client(c.sock, buffer);

}

int update_lobby(Client *clients, Client **enAttente, int actual, fd_set *ptr)
{
    int i;
    int j = 0;
    for (i = 0; i < actual; i++) // Ajout des clients hors partie dans la liste des joueurs du lobby
    {

        if (clients[i].partie == -1) // Si le joueur n'est pas dans une partie, il sera dans le lobby (liste enAttente)
        {
            enAttente[j] = &clients[i]; // remplissage de la variable des joueurs en attente
            FD_SET(enAttente[j]->sock, ptr);
            j++;
        }
    }
    return j;

}

static void remove_client(Client *clients, int to_remove, int *actual)
{

/* we remove the client in the array */

    memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));

/* number client - 1 */

    *actual -= 1;
}

int read_client(SOCKET sock, char *buffer)
{
    int n = 0;

    if ((n = (int) recv(sock, buffer, BUF_SIZE, 0)) < 0)
    {
        perror("read()");
        //si erreur on déconnecte le client
        close(sock);
        printf("Client déconnecté!\n");
        return -1;
    }
    buffer[n] = '\0';

    return n;
}

int write_clients(Client c, Client *clients, int max, char *buffer)
{
    char message[BUF_SIZE];
    sprintf(message, "%s : ", c.name);
    strcat(message, buffer);

    for (int i = 0; i < max; i++)
    {
        if (clients[i].id != c.id)
        {
            if (write_client(clients[i].sock, message) == -1)
            {
                remove_client(clients, i, &max);
                return -1;
            }
        }
    }
    return 1;
}

int nouvelle_connexion(Client c, Client *clients, int max)
{
    char message[BUF_SIZE];
    sprintf(message, "%s ", c.name);
    strcat(message, msgConnexion);

    for (int i = 0; i < max; i++)
    {
        if (clients[i].id != c.id)
        {
            if (write_client(clients[i].sock, message) == -1)
            {
                remove_client(clients, i, &max);
                return -1;
            }
        }
    }
    return 1;
}

int write_client(SOCKET sock, const char *buffer)
{
    int n = (int) send(sock, buffer, BUF_SIZE, 0);
    if (n == -1)
    {
        fprintf(stderr, "Erreur fonction->write_client: write");
        close(sock);
        return -1;
    }
    return n;
}


int init_connection(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin;

    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
    {
        perror("bind()");
        exit(errno);
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

void extinction(int sig)
{
    exit(sig);
}

int estFinStr(char i)
{
    if (i == '\n' || i == '\0' || i < 0) return 1;
    return 0;
}

int main()
{
    SOCKET sock = init_connection(); // Initialisation du serveur TCP

    // Variables

    Client clients[MAX_CLIENTS]; // Liste de tous les clients
    Client *enAttente[MAX_CLIENTS]; // Liste des clients qui ne sont pas dans une partie
    int id = 0; // id de chaque joueur (unique)


    int actual = 0; // Nombre de clients connectés
    int max = sock; // pour calculer le select du lobby

    // Variables de parties
    int partieId = 0; // Id de la partie (unique)
    int pip[MAX_GAMES][2]; // array pour les tubes des fils
    int partieN = 0; //Numéro de l'array int du tube utilisé par le fils (1 fils = 1 partie) ex. pip[partieN]
    data parties[MAX_GAMES]; //Envoi des info nécessaires à la méthode partie(data a) et retour en fin de partie
    FILE *log = NULL; // descripteur pour cibler fichier des scores de parties

    char buffer[BUF_SIZE];

    fd_set rdfs; // descripteur de lecture pour la fonction select()


    /* ------------------- PREPARATION DU SIGNAL ---------------------------------*/
    signal(SIGTERM, extinction);


    /*---------------------LOBBY (SALLE D'ATTENTE)------------------------------*/
    while (1)
    {
        int i; // pour les boucles for
        int j; // pour compter les joueurs présents dans le lobby (i.e. hors d'une partie)

        FD_ZERO(&rdfs); // Mise à zéro du descripteur lecture

        FD_SET(sock, &rdfs); // Ajout sock au descripteur de lecture pour connexion de clients

        j = update_lobby(clients, enAttente, actual, &rdfs);

        for (i = 0; i < MAX_GAMES; i++) // Ajout des tubes créés au descripteur
        {
            if (pip[i][0] != NULL)
            {
                FD_SET(pip[i][0], &rdfs);
                printf("Select() sur pip%d\n", i);
                max = pip[i][0] > max ? pip[i][0] : max;
            }

        }

        /*-------------------SELECTEUR D'EVENT ----------------------------------------*/

        if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1) // FONCTION SELECT
            /* CAS SOCK = connexion d'un client
             * CAS SOCK CLIENT = Entrée clavier client
             *          ==> Actualisation de la liste des joueurs en attente
             *          ==> Deconnexion du serveur
             *          ==> Lancement d'une partie avec un joueur du lobby
             * CAS PIPE = fin d'une partie, reception résultats, extinction du fils
             */
        {
            perror("select()");
            exit(errno);
        } else if (FD_ISSET(sock, &rdfs))
        {

            /* ------------ CONNEXION NOUVEAU CLIENT ------------------*/
            SOCKADDR_IN csin;
            socklen_t sinsize = sizeof csin;
            int csock = accept(sock, (SOCKADDR *) &csin, &sinsize);
            if (csock == SOCKET_ERROR)
            {
                perror("accept()\n");
                continue;
            }

            /* après connexion, le client envoie son pseudo */
            if (read_client(csock, buffer) == -1)
            {
                /* deconnexion */
                continue;
            }
            puts(buffer);

            /* calcul du fd maximum */
            max = csock > max ? csock : max;
            printf("new max\n");
            FD_SET(csock, &rdfs);

            /* enregistrement du nouveau client dans la liste client */
            Client c = {csock};
            strncpy(c.name, buffer, BUF_SIZE - 1);
            c.partie = -1;
            c.id = id;
            clients[actual] = c;

            actual++;
            id++;

            printf("new client, name = %s\n", c.name);

            /* enregistrement dans la liste enAttente */
            j = update_lobby(clients, enAttente, actual, &rdfs);


            write_client(c.sock, msgAccueil);
            message_lobby(c, *enAttente, j); // affichage du message d'acceuil et de la liste des joueurs au client
            nouvelle_connexion(c, clients, actual);

        } else
        {
            for (i = 0; i < j; i++) //Passe en revue tous les clients qui ne jouent pas de la liste enAttente
            {
                /* ------------ ENVOI D'UN MESSAGE PAR UN CLIENT DU LOBBY ------------------*/
                if (FD_ISSET(enAttente[i]->sock, &rdfs))
                {
                    printf("Lecture clavier utilisateur\n");
                    Client client = *enAttente[i];
                    int posClient = rechercher_joueur(clients, client.id,
                                                      actual); // position du client dans la liste clients[]

                    int c = read_client(client.sock, buffer); // ecriture dans le buffer
                    if (c == 0) // si vide, erreur => client déconnecté => suppression du client
                    {
                        close(client.sock);
                        remove_client(clients, posClient, &actual); // enlève le client de la liste clients
                    } else
                    {

                        /* ------------ INPUT = A ==> ACTUALISATION  ------------------*/
                        if ((buffer[0] == 'A' || buffer[0] == 'a') && (estFinStr(buffer[1])))
                        {
                            //Message du lobby avec liste joueur
                            message_lobby(client, *enAttente, j);


                        } else if ((buffer[0] == 'Q' || buffer[0] == 'q') && (estFinStr(buffer[1])))
                        {
                            /* ------------ INPUT = Q ==> DECONNEXION  ------------------*/

                            write_client(client.sock, msgAuRevoir);
                            close(clients[posClient].sock);
                            sprintf(buffer, "%s s'est déconnecté !\n", client.name);
                            write_clients(client, *enAttente, j, buffer);
                            remove_client(clients, posClient, &actual);
                        } else
                        {
                            /* ------------ INPUT = Nombre ==> PARTIE CONTRE Nombre  ------------------*/

                            int isnumber = 0;
                            for (int k = 0; k < strlen(buffer); k++)
                            {
                                if (!isdigit(buffer[k])) isnumber++;
                            }
                            if (isnumber == 0)  // Si le buffer est un nombre (et seulement un nombre)
                            {
                                long choix = strtol(buffer, NULL, 10) - 1; // conversion en long

                                if (choix == i)
                                {
                                    sprintf(buffer, "Vous ne pouvez pas vous choisir vous-même ! HAhahaahaha !\n");
                                    write_client(client.sock, buffer);
                                    message_lobby(client, *enAttente, j); // affichage du message d'acceuil et de la liste des joueurs au client

                                } else if (choix >= 0 && choix < j) // Si le choix est bien dans la variable enAttente
                                {
                                    /* ------------ MISE EN PLACE PARTIE  ------------------*/

                                    /* Vérification qu'il reste des tubes libres */
                                    partieN = -1;
                                    for (int t = MAX_GAMES - 1; t >= 0; t--)
                                    { if (pip[t][0] == NULL) partieN = t; }

                                    if (partieN > -1) // Il reste au moins un tube libre pour créer une partie
                                    {
                                        enAttente[i]->partie = partieId; // On actualise la partie du joueur => il va quitter le lobby
                                        enAttente[choix]->partie = partieId; // On actualise la partie du joueur => il va quitter le lobby


                                        /* On enlève les sock des 2 joueurs de la liste du descripteur select() */
                                        FD_CLR(enAttente[i]->sock, &rdfs);
                                        FD_CLR(enAttente[choix]->sock, &rdfs);


                                        /* Données à passer à la méthode partie(data a) */
                                        parties[partieId].p1 = *enAttente[i];
                                        parties[partieId].p2 = *enAttente[choix];
                                        parties[partieId].partieId = partieId;
                                        parties[partieId].partieN = partieN;


                                        /* création du tube */
                                        if (pipe(pip[partieN]) == -1)
                                        {
                                            printf("erreur de pipe\n");
                                            return 1;
                                        }

                                        /* FORK */
                                        int pid;
                                        if ((pid = fork()) < 0)
                                        {
                                            printf("erreur fork\n");
                                            return 2;
                                        }

                                        if (pid == 0)
                                        {
                                            /* fils */
                                            close(pip[partieN][0]);// fermer Read sur tube

                                            data resultats = partie(
                                                    parties[partieId]); // Partie et récupération résultats

                                            write(pip[resultats.partieN][1], &resultats,
                                                  sizeof(resultats)); // envoi résutalts sur tube

                                            sleep(3); // temps que le père puisse bien lire le tube
                                            close(pip[resultats.partieN][1]); // fermeture tube
                                            exit(EXIT_SUCCESS); //extinction fils

                                        } else
                                        {
                                            /* père */
                                            close(pip[partieN][1]);// fermer Write sur tube
                                            partieId++; // incrémentation partie ID

                                        }


                                    } else //Plus de tubes disponibles
                                    {
                                        strcpy(buffer, msgTropdeParties);
                                        write_client(client.sock, buffer);
                                    }
                                }
                            } else
                            { // autre saisie du client du lobby
                                /* ------------------------ FONCTION CHAT -----------------------------*/
                                puts(buffer);
                                write_clients(client, *enAttente, j,
                                              buffer); // Envoi du message aux autres clients du lobby (chat)
                            }
                        }
                    }
                }
            }
            for (i = 0; i < MAX_GAMES; i++)
            {
                /*------------------- VERIFICATION SORTIE PIPE = resulats d'une partie ----------*/
                /* Si un pipe est activé par la fin d'un processus fils (partie) */
                if (pip[i][0] != NULL)
                {
                    if (FD_ISSET(pip[i][0], &rdfs)) // Lecture du descripteur
                    {
                        int status;
                        data resultatsPere;
                        read(pip[i][0], &resultatsPere, sizeof(resultatsPere)); // récupération en sortie pipe

                        int idJ1 = rechercher_joueur(clients, resultatsPere.p1.id, actual);
                        int idJ2 = rechercher_joueur(clients, resultatsPere.p2.id, actual);

                        close(pip[i][0]); // fermeture du pipe
                        waitpid(resultatsPere.pid, &status, 0); // fermeture propre du fils
                        pip[i][0] = NULL; // pipe mis à nul (pour les conditions utilisées plus haut)
                        printf("%s et %s\n", resultatsPere.p1.name, resultatsPere.p2.name);

                        if (resultatsPere.partieId == 666)
                        {
                            close(resultatsPere.p1.sock);
                            close(resultatsPere.p2.sock);
                            remove_client(clients, idJ1, &actual);
                            remove_client(clients, idJ2, &actual);
                        } else
                        {

                            //Les clients sont à présents hors partie. Ils seront rajoutés à enAttente
                            clients[idJ1].partie = -1;
                            clients[idJ2].partie = -1;
                            j = update_lobby(clients, enAttente, actual, &rdfs);
                            write_client(clients[idJ1].sock, msgAccueil);
                            message_lobby(clients[idJ1], *enAttente, j);
                            write_client(clients[idJ2].sock, msgAccueil);
                            message_lobby(clients[idJ2], *enAttente, j);



                            /* --------------- ECRITURE DU SCORE DANS UN FICHIER LOG PERMANENT -----------------*/
                            log = fopen("Score.log", "a"); // a = Ecriture en fin de fichier

                            if (log != NULL)
                            {
                                Client joueur[MAX_CLIENTS_JEU];
                                joueur[0] = resultatsPere.p1;
                                joueur[1] = resultatsPere.p2;

                                buffer[0] = '\0';
                                char temp[36];
                                for (int f = 0; f < MAX_CLIENTS_JEU; f++)
                                {
                                    if (resultatsPere.gagnant == f) sprintf(temp, " - [%s]", joueur[f].name);
                                    else sprintf(temp, "- %s ", joueur[f].name);
                                    strcat(buffer, temp);

                                }

                                fprintf(log, "Jeu n.%d%s\n", resultatsPere.partieId, buffer);
                                fclose(log);
                            }
                        }
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}








