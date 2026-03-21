/*
 * echoclient.c - An echo client
 */
#include "csapp.h"

int main(int argc, char **argv)
{   
    //initialisations de variables
    int clientfd, port, n;
    char *host, buf[MAXLINE], *nomfic;
    char chemin[MAXLINE+10] ;
    char cmd[MAXLINE];

    //gestion des arguments
    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <nomfic>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    nomfic = argv[2];

    //port par défaut
    port = 2121;

    //connection au serveur
    clientfd = Open_clientfd(host, port);
    fprintf(stdout,"client connected to server OS\n"); 

   
    //récupération du nom du fichier
    sprintf(cmd, "%s\n", nomfic);
    Rio_writen(clientfd, cmd, strlen(cmd));

    //pour écrire dans dossier où on recoit les données
    sprintf(chemin, "./DATA/%s", nomfic);

    //ouverture/création du fichier reçu
    int fd = open(chemin, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    
    //lecture du buffer renvyé par le serveur
    while ((n = Rio_readn(clientfd, buf, MAXLINE)) > 0) {
        Rio_writen(fd, buf, n);//écriture du contenu du fichier dans le fichier recu
    }

    //fermeture des fichiers
    Close(fd);
    Close(clientfd);
    exit(0);
}