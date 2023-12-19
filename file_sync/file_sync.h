//  Partie interface du module file_sync (File synchronisée).
//
//  Une file synchronisée permet est une file qui à la particularité d'être
//    bloquante lorque le file est plein et que l'on veut ajouter ou vide et
//    l'on veut retirer un élément. Chaque ajout ou retrait ce fait de maniére
//    atomique (on ne peut retirer qu'un seul ou ajouter qu'un seul éléments
//    à chaque opération)

#ifndef SLOWER__H
#define SLOWER__H

#include <stdlib.h>
//  create_file_sync : crée un segment de mémoire partagée contenant les
//    éléments permettant de gérer la file synchronisée. Renvoi -1 si il y a une
//    erreur lors de la création de la file, 0 sinon.
extern int create_file_sync(void);

//  destroy_file : libére les ressources allouées et supprime les sémaphores et
//    le segment de mémoire partagée. Renvoi -1 si il y a une erreur lors de la
//    libération de la file, 0 sinon.
extern int destroy_file(void);

//  enfiler : ajoute un élément à la file synchronisée. Renvoi -1 si il y a une
//    erreur lors de la gestion des sémaphores, 0 sinon.
extern int enfiler(pid_t donnee);

// defiler : retire un élément à la file synchronisée. Renvoi -1 si il y a une
//    erreur lors de la gestion des sémaphores, 0 sinon.
extern pid_t defiler(void);
#endif
