//
// Created by larnal on 01/01/18.
//

#ifndef PUISSANCE4CLIENT_A_H
#define PUISSANCE4CLIENT_A_H

#include "affichage.h"

int lire(char *chaine, size_t longueur);
void viderBuffer();
int deserialize_jeu(char *buffer, Jeux jeu);
char *substring(const char *s, unsigned int start, unsigned int end);



#endif //PUISSANCE4CLIENT_A_H
