#include "csapp.h"
#include "structure.h"


int traitement(int connfd)
{   
    //Initialisation de variables
    request_t requete;
    response_t reponse;
    char buf[MAXLINE];
    char chemin[MAXLINE + 10];
    rio_t rio_client;
    int fd;
    ssize_t n;
    int w,i;

    Rio_readinitb(&rio_client, connfd);
    
    // lecture de la requête  
    if (Rio_readnb(&rio_client, &requete, sizeof(requete)) <= 0) return 0;

    //Si la requête envoyée est invalide
    if ( requete.type == INCONNU){
        reponse.type = ERREUR_REQ;
        Rio_writen(connfd,&reponse,sizeof(reponse)); //Renvoi de l'erreur
        return 1;
    }

    else if (requete.type == GET){ //la requête reçue est un get
        //les fichiers sont stockés dans le dossier DATA du serveur
        //permet d'éviter d'accéder au code du serveur 
        sprintf(chemin, "./DATA/%s", requete.fic);

        //ouverture du fichier demandé
        if ((fd = open(chemin, O_RDONLY, 0)) < 0) {
            reponse.type = FICHIER_INEXISTANT; //Si le fichier n'a pas été trouvé
            Rio_writen(connfd,&reponse,sizeof(reponse)); //Renvoi de l'erreur
            return 1;
        }

        //calcul de la taille du fichier à envoyer
        int taille = lseek(fd, 0, SEEK_END);
        lseek(fd, requete.numero * TAILLESOCKET, SEEK_SET);
        w = requete.numero;

        reponse.type = SUCCES;
        //calcul du nombre de packets à envoyer au client pour transférer le fichier 
        if (taille%TAILLESOCKET==0){
            reponse.total = taille / TAILLESOCKET; 
        }
        else {
            reponse.total = taille / TAILLESOCKET +1;
        }
        Rio_writen(connfd, &reponse, sizeof(reponse)); //Renvoit en réponse un succès au client ainsi que la taille du fichier
        i = w;

        while((n = Rio_readn(fd,buf,TAILLESOCKET)) > 0){ //envoi socket par socket
            reponse.numero = i; //stocke le numéro de la socket
            reponse.taille = n; //stocke la taille de notre socket
            Rio_writen(connfd,&reponse,sizeof(reponse)); //envoie les informations sur la socket
            Rio_writen(connfd, buf, n); //envoie la socket
            i++;
        }    
        Close(fd);
        return 1;
    }
    else if (requete.type == LS){ //Traitement d'une requête LS
         
        int fd_pipe[2];
        pipe(fd_pipe);
        if (Fork()==0){ //Créer un fils pour ne pas tuer l'esclave
            close(fd_pipe[0]); //Ferme la lecture
            dup2(fd_pipe[1],STDOUT_FILENO); //Remplace la sortie standard par l'entrée écriture du pipe
            close(fd_pipe[1]); //Ferme l'écriture
            char *arg[4];

            if (strlen(requete.fic)>0){ //Si ls a un argument
                arg[0]="ls";
                arg[1]=requete.fic;
                arg[2]="DATA";
                arg[3]=NULL;
            }
            else { //Si ls n'a pas d'argument
                arg[0]="ls";
                arg[1]="DATA";
                arg[2]=NULL;
            }
            execvp(arg[0],arg); //Execute la commande ls avec (si présent) ses arguments
        }
        else{
            close(fd_pipe[1]); //Ferme l'écriture

            reponse.type = SUCCES;
            Rio_writen(connfd,&reponse,sizeof(reponse)); //Envoie une réponse de type succès
            reponse.numero = 1;
            reponse.total = 1;
            //on envoie le résultat du ls en une fois quoi qu'il arrive
            while((n = Rio_readn(fd_pipe[0],buf,MAXLINE)) > 0){
                reponse.taille = n;

                Rio_writen(connfd,&reponse,sizeof(reponse)); //Envoie une réponse
                Rio_writen(connfd, buf, n); //Envoie le résultat du ls 
            }

            close(fd_pipe[0]); //Ferme la lecture
            Wait(NULL); //Attend la fin du fils
            
        }
    }
    return 1;
    
}