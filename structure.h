#include "csapp.h"

#ifndef REQUETE_H
#define REQUETE_H

#define TAILLENOM 500
typedef enum {
    GET,
    PUT,
    LS
}typereq_t;

typedef struct 
{
   typereq_t type;
   char fic[TAILLENOM];
}request_t;  

typedef enum{
    SUCCES,
    FICHIER_INEXISTANT,
    ERREUR_REQ
}type_rep;

typedef struct{
    type_rep type;
}response_t;


#endif