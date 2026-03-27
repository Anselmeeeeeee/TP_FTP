/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include "structure.h"


int cat(int connfd)
{   
    request_t requete;
    response_t reponse;
    char buf[MAXLINE];
    char chemin[MAXLINE + 10];
    rio_t rio_client;
    int fd;
    ssize_t n;
    int w,i;

    Rio_readinitb(&rio_client, connfd);
    
    // lecture du nom du fichier 
    if (Rio_readnb(&rio_client, &requete, sizeof(requete)) <= 0) return 0;

    if ( requete.type == INCONNU){
        reponse.type = ERREUR_REQ;
        Rio_writen(connfd,&reponse,sizeof(reponse));
        return 1;
    }

    //les fichiers sont stockés dans le dossier DATA du serveur
    sprintf(chemin, "./DATA/%s", requete.fic);
    //printf("%s",chemin);

    //ouverture du fichier 
    if ((fd = open(chemin, O_RDONLY, 0)) < 0) {
        reponse.type = FICHIER_INEXISTANT;
        Rio_writen(connfd,&reponse,sizeof(reponse));
        return 1;
    }

    int taille = lseek(fd, 0, SEEK_END);
    //printf("requete.numero : %d, requete.index : %d", requete.numero, requete.index);
    lseek(fd, requete.numero * TAILLESOCKET, SEEK_SET);
    w = requete.numero;

    //Renvoie la réponse cas succes
    reponse.type = SUCCES;
    if (taille%TAILLESOCKET==0){
        reponse.total = taille / TAILLESOCKET; 
    }
    else {
        reponse.total = taille / TAILLESOCKET +1;
    }
    Rio_writen(connfd, &reponse, sizeof(reponse));
    i = w;

    while((n = Rio_readn(fd,buf,TAILLESOCKET)) > 0){
        reponse.numero = i;
        reponse.taille = n;
        Rio_writen(connfd,&reponse,sizeof(reponse));
        Rio_writen(connfd, buf, n);
        i++;
    }    
    Close(fd);
    return 1;
}