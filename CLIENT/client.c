/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "structure.h"
int main(int argc, char **argv)
{   
    //initialisations de variables
    int clientfd, port, n;
    char *host, buf[MAXLINE];
    char chemin[MAXLINE+10] ;
    char cmd[MAXLINE];
    request_t requete;
    char req[TAILLENOM];

    //gestion des arguments
    if (argc != 3) {
        fprintf(stderr, "usage: %s <action> <nomfic>\n", argv[0]);
        exit(0);
    }
    host = "localhost"; //défaut
    
    //attribution du type de la commande
    strcpy(req, argv[1]);
    if (strcmp(req,"get")==0 ){
        requete.type = GET;
    }
    else if (strcmp(req,"put")==0 ){
        requete.type = PUT;
    }
    else if (strcmp(req,"ls")==0 ){
        requete.type = LS;
    }
    else { // ne correspond à aucune requete
        printf("TYPE DE REQUETE INCONNUE, PEUT ETRE A CHANGER\n");
    }
    strcpy(requete.fic,argv[2]);

    //port par défaut
    port = 2121;

    //connection au serveur
    clientfd = Open_clientfd(host, port);
    fprintf(stdout,"client connected to server OS\n"); 

    //récupération du nom du fichier
    sprintf(cmd, "%s\n", requete.fic);

    strcpy(requete.fic,cmd);

    Rio_writen(clientfd, &requete, sizeof(requete));

    //pour écrire dans dossier où on recoit les données
    sprintf(chemin, "./DATA/%s", requete.fic);

    //ouverture/création du fichier reçu
    int fd = open(requete.fic, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    
    //lecture du buffer renvyé par le serveur
    while ((n = Rio_readn(clientfd, buf, MAXLINE)) > 0) {
        Rio_writen(fd, buf, n);//écriture du contenu du fichier dans le fichier recu
    }

    //fermeture des fichiers
    Close(fd);
    Close(clientfd);
    exit(0);
}