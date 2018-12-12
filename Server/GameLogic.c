//
// Created by larnal on 27/11/17.
//

#include <printf.h>
#include "GameLogic.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>



//------------------------------//
//--------CONSTANTES------------//
//------------------------------//

int colonnePleine(Jeux jeu, int col)
{
    //Teste si la case n'est pas vide (return true) ou pleine (return false)

    // si la dernière case de la colonne est vide, return = false (pas plein)
    // sinon  return true (nécéssairement plein)

    if (jeu[col][0] == 0) {
        return 0;
    } else {
        return 1;
    }
}


int plateauPlein(Jeux jeu)
{

    //Vérifie si le plateau est plein et renvoie true si plein

    // Boucle for : si une des colonne pas pleine, on renvoie de suite false.
    // Sinon on vérifie chaque colonne, si toutes pleines, on renvoie true.
    for (int x = 0; x < NCOL; x++) {
        if (!colonnePleine(jeu, x)) {
            return 0;
        }
    }
    return 1;
}


int initJeu(Jeux jeu)
{

    //Ecrit "vide" dans toutes les cases du plateau

    for (int y = 0; y < NLINE; y++) {
        for (int x = 0; x < NCOL; x++) {
            jeu[x][y] = 0;
        }
    }
    return 1;
}


int jouerPion(Jeux jeu, int col, int joueur)
{

    //Ajoute un pion du joueur au plateau à la colonne indiquée

    //renvoie la ligne à laquelle a été placée le pion

    for (int y = NLINE - 1; y >= 0; y--) {
        if (jeu[col][y] == 0) {
            jeu[col][y] = joueur + 1;
            return y;
        }
    }
    // Si aucun pion n'a pu être placé, c'est une erreur
    return -1;
}


int verifPion(Jeux jeu, coord sens, coord position, int joueur)
{

    // Teste si 4 pions du joueur courant sont alignés.
    // Le sens de recherche dépend de sens
    // En suivant le sens, la position de vérification avance jusqu'au dernier pion du joueur courant.
    // Puis le sens est inversé, la position est incrémentée et le compteur comptePionJoueur s'incrémente si le pion appartient au joueur courant.
    // Si le compteur comptePionJoueur atteint 4, renvoie TRUE. Sinon renvoie FALSE.

    int comptePionJoueur = 0; // Compteur de pions successifs du joueur courant

    // on met les coord de la position à vérifier à coord. et on décale d'une itération.
    position.x += sens.x;
    position.y += sens.y;

    // boucle pour avancer jusqu'au bout de la succession de pion du joueur courant
    while (jeu[position.x][position.y] == (joueur + 1) &&
           ((position.x >= 0 && position.x < NCOL) && (position.y >= 0 && position.y < NLINE))) {
        position.x += sens.x;
        position.y += sens.y;
    }

    //inversion du sens de recherche
    sens.x = -sens.x;
    sens.y = -sens.y;

    position.x += sens.x;
    position.y += sens.y;

    //boucle pour compter le nombre de pions successifs du joueur courant
    while (jeu[position.x][position.y] == (joueur + 1) &&
           ((position.x >= 0 && position.x < NCOL) && (position.y >= 0 && position.y < NLINE))) {
        comptePionJoueur += 1;
        position.x += sens.x;
        position.y += sens.y;

    }

    // si le compteur atteint 4, renvoie TRUE
    if (comptePionJoueur >= 4) {
        return 1;
    }
    // Il n'y a pas 4 pions successifs du joueur courant, renvoie FALSE
    return 0;
}


int pionGagnant(Jeux jeu, coord coordo, int joueur)
{

    //Teste si le pion permet de gagner.
    // Renvoie TRUE si le pion gagne la partie pour le joueur courant
    // Renvoie FALSE sinon

    //sensRecherche[] : permet l'incrémentation de la position pour vérifier le pions
    // Vertical - Horizontal - Diagonale BasDroite - Diagonale HautDroite

    coord sensRecherche[4] = {{0, 1},
                              {1, 0},
                              {1, 1},
                              {1, -1}};

    for (int i = 0; i < 4; i++) {
        if (verifPion(jeu, sensRecherche[i], coordo, joueur)) {
            return 1;
        }
    }
    return 0;
}
