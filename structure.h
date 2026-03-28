#include "csapp.h"

#ifndef REQUETE_H
#define REQUETE_H

#define TAILLESOCKET 1024
#define TAILLENOM 500
typedef enum {
    GET,
    PUT,
    LS,
    INCONNU
}typereq_t;

typedef struct 
{
   typereq_t type;
   char fic[TAILLENOM];
   int numero;//numero du paquet
   int index;//indice dans le paquet au moment où ca a crash (pas sur de la nécessité)
}request_t;  

typedef enum{
    SUCCES,
    FICHIER_INEXISTANT,
    ERREUR_REQ
}type_rep;

typedef struct{
    int numero;
    int total;
    int taille;
    type_rep type;
}response_t;

typedef enum{
    OUI,
    NON
}disponibilite;

typedef struct{
    int port;
    disponibilite dispo;
    int sockfd;
}esclave_t;



#endif