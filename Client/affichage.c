//
// Created by larnal on 30/12/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "affichage.h"
#include "InputManage.h"



void afficher(Jeux jeu)
{
// A compléter avec interface graphique pour le client (pas le serveur)
    const char lettrejoueur[4] = {'.', 'X', 'O','\0'};


    // Bord supérieur du plateau
    printf("+");
    for (int i = 0; i < NCOL; i++) {
        printf("++");
    }
    printf("\n");

    //boucle pour afficher l'intérieur
    for (int i = 0; i < NLINE; i++) { // Parcourt les lignes (y)

        printf("|"); // bord gauche
        for (int j = 0; j < NCOL; j++) { //Parcourt les colonnes (x)
            printf("%c|", lettrejoueur[jeu[j][i]]);
        }
        printf("\n");

        //ligne séparatrice entre 2 lignes de pions
        printf("+");
        for (int a = 0; a < NCOL; a++) {
            printf("++");
        }
        printf("\n");

    }

}
