//
// Created by larnal on 11/12/17.
//


#ifndef PUISSANCE4SERVER_GAMELOGIC_H
#define PUISSANCE4SERVER_GAMELOGIC_H

#define NLINE 6
#define NCOL 7

typedef struct coord {
    int x;
    int y;
} coord;

typedef int Jeux[NCOL][NLINE];

/* METHODES */
int colonnePleine(Jeux jeu, int col);
int plateauPlein(Jeux jeu);
int initJeu(Jeux jeu);
int jouerPion(Jeux jeu, int col, int joueur);
int pionGagnant(Jeux jeu, coord coordo, int joueur);
int verifPion(Jeux jeu, coord sens, coord position, int joueur);


#endif //PUISSANCE4SERVER_GAMELOGIC_H
