#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "main.h"
#include "InputManage.h"


int init_connection(const char *adresse)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN sin;

    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(errno);
    }

    memset(&sin, 0, sizeof(struct sockaddr_in));
    inet_aton(adresse, &sin.sin_addr);
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if (connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()\n");
        exit(errno);
    }

    return sock;
}


void end_connection(int sock)
{
    close(sock);
    printf("Fermeture connexion.\n");
}


int lecture_serveur(SOCKET sock, int *boucle)
/* MODE BLOQUANT de la communication avec le serveur
 * 3 modes de communications :
 *  - AfficherTexte = Affiche le texte envoyé par le serveur
 *  - AfficherJeu = Affichage du plateau de jeu dans la console
 *  - DemandeInput = Affiche le texte du serveur PUIS prend une saisie clavier qui sera envoyée au serveur
 *  - FinJeu = Affiche le message du serveur et quitte la boucle de SAISIE BLOQUANTE
 */
{
    Jeux jeu = {0};
    char buffer[BUF_SIZE];
    int n = read_server(sock, buffer);
    int typeMessage = (buffer[0] - '0'); //Premier caractère du message du serveur défini le type de message (parmis 4)

    switch (typeMessage)
    {
        case 3: //FinJeu
            *boucle = 0;

        case 1: //AfficherTexte
            strcpy(buffer, substring(buffer, 1, (int) strlen(buffer)));
            printf("%s\n", buffer);
            break;

        case 0://DemandeInput
            strcpy(buffer, substring(buffer, 1, (int) strlen(buffer)));
            printf("%s\n", buffer);
            char output[BUF_SIZE];
            lire(output, BUF_SIZE - 1);
            write_server(sock, output);
            break;

        case 2: //AfficherJeu
            deserialize_jeu(substring(buffer, 1, (int) strlen(buffer)), jeu);
            afficher(jeu);
            break;

        default:
            printf("Erreur lecture typeMessage.\n");
            end_connection(sock);
            break;
    }

    return n;
}


static int read_server(SOCKET sock, char *buffer)
{
    int n = 0;

    if ((n = (int) recv(sock, buffer, BUF_SIZE, 0)) < 0)
    {
        perror("read() error");
        exit(errno);
    }

    buffer[n] = '\0'; // Termine le string par un EOF

    return n;
}

void write_server(SOCKET sock, const char *buffer)
{
    if (send(sock, buffer, BUF_SIZE, 0) < 0)
    {
        perror("send()");
        exit(errno);
    }

}


int main()
{

    char adresseServeur[16]; // Adresse du serveur de la forme IP4 xxx.xxx.xxx.xxx
    char pseudo[30]; // Pseudo (max 29 caractères

    fd_set rdfs; // Descripteur pour la fonction select()

    char buffer[BUF_SIZE];

    /* --- Récupération ADRESSE Serveur _________*/
    printf("Veuillez entrer l'adresse du serveur. \n");
    if (!lire(adresseServeur, sizeof(adresseServeur)))
    {
        printf("Erreur dans l'entrée de l'adresse : %s \n", adresseServeur);
        return EXIT_FAILURE;
    }

    /* --- Récupération PSEUDO _________*/
    printf("Veuillez entrer votre pseudo. \n");
    if (!lire(pseudo, sizeof(pseudo)))
    {
        printf("Erreur dans l'entrée du pseudo : %s \n", pseudo);
        return EXIT_FAILURE;
    }

    /*-------- CONNEXION ----------------*/
    SOCKET sock = init_connection(adresseServeur);
    /* ----------ENVOI PSEUDO -------------*/
    write_server(sock, pseudo);


    while (1)
    {
        FD_ZERO(&rdfs); // Reset du descripteur select()



        /* Ajout STDIN (entrée clavier) au descripteur */
        FD_SET(STDIN_FILENO, &rdfs);

        /* Ajout Socket (serveur) au descripteur */
        FD_SET(sock, &rdfs);

        if (select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            /* CAS STDIN = entrée clavier => envoi au serveur
            * CAS SOCK = Réception du serveur => Affichage
            *          ==> Actualisation de la liste des joueurs en attente
            *          ==> Deconnexion du serveur
            *          ==> Lancement d'une partie avec un joueur du lobby
            * CAS PIPE = fin d'une partie, reception résultats, extinction du fils
            */
            perror("select()");
            exit(errno);
        }

        /*---- EVENT = ENTREE CLAVIER------ */
        if (FD_ISSET(STDIN_FILENO, &rdfs))
        {
            fgets(buffer, BUF_SIZE - 1, stdin);
            {
                char *p = NULL;
                p = strstr(buffer, "\n"); // Si pas de \n à la fin, on va rajouter un \0 à la fin du buffer
                if (p != NULL)
                {
                    *p = 0;
                } else
                {
                    /* fclean */
                    buffer[BUF_SIZE - 2] = '\0';
                }
            }
            write_server(sock, buffer); // Envoi au serveur
        } else if (FD_ISSET(sock, &rdfs))
        {
            /*---- EVENT = RECEPTION DEPUIS SERVEUR------ */

            int n = read_server(sock, buffer);
            /* server down */
            if (n == 0)
            {
                printf("Server disconnected !\n");
                end_connection(sock);
                return EXIT_SUCCESS;
            }
            if (strcmp(buffer, startgame) == 0) // Si le serveur envoie STARTGAME, ==> MODE BLOQUANT de communication
            {
                FD_ZERO(&rdfs); //On efface les descripteurs du select()
                sleep(1); // On attend 1 sec que ce soit fait
                int boucleJeu = 1;
                while (boucleJeu)
                {
                    if ((lecture_serveur(sock, &boucleJeu) == -1))
                    { //Lecture/Ecriture MODE BLOQUANT (on est dans une partie)
                        printf("Server disconnected !\n");
                        boucleJeu = 0;
                    }
                }

            } else puts(buffer); // Si le message du serveur n'est pas STARTGAME, on l'affiche
        }
    }


    return EXIT_SUCCESS;
}