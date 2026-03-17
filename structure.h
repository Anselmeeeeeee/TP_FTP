#include "csapp.h"

#ifndef REQUETE_H
#define REQUETE_H

typedef enum {
    GET,
    PUT,
    LS
}typereq_t;

typedef struct 
{
   typereq_t type;
   char *fic;
}request_t;  

typedef enum{
    SUCCES,
    FICHIER_INEXISTANT,
    ERREUR_REQ
}response_t;


#endif