/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include "structure.h"


void cat(int connfd)
{
    char buf[MAXLINE];
    char chemin[MAXLINE + 10];
    rio_t rio_client;
    int fd;
    ssize_t n;

    Rio_readinitb(&rio_client, connfd);
    
    // lecture du nom du fichier 
    if (Rio_readlineb(&rio_client, buf, MAXLINE) <= 0) return;

    // nettoyage du nom
    size_t l = strlen(buf);
    if (l > 0 && buf[l-1] == '\n') buf[l-1] = '\0';

    //les fichiers sont stockés dans le dossier DATA du serveur
    sprintf(chemin, "./DATA/%s", buf);


    //ouverture du fichier 
    if ((fd = open(chemin, O_RDONLY, 0)) < 0) {
        fprintf(stdout,"Impossible d'ouvrir le fichier\n");
        return;
    }

    // écrit dans un buffer le contenu du fichier
    while ((n = Rio_readn(fd, buf, MAXLINE)) > 0) {
        Rio_writen(connfd, buf, n); 
    }
    
    Close(fd);
}