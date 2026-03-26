/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "structure.h"

int main(int argc, char **argv)
{   
    //initialisations de variables
    int clientfd, port;
    char *host, buf[MAXLINE];
    char chemin[MAXLINE+10] ;
    request_t requete;
    response_t reponse;
    char req[TAILLENOM];
    rio_t rio_client ;
    int fd;
    int flag;
    int offset;
    char ligne[MAXLINE];
    

    //gestion des arguments
    if (argc != 1) {
        fprintf(stderr, "usage: %s \n", argv[0]);
        exit(0);
    }
    host = "localhost"; //défaut
    
    //port par défaut
    port = 2121;

    //connection au serveur
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio_client, clientfd);
    fprintf(stdout,"client connected to server OS\n"); 
    
    fprintf(stdout,"ftp>");
    while(fgets(ligne,MAXLINE,stdin)){
        memset(&requete,0,sizeof(requete));

        if (strlen(ligne)>=3){
            if (ligne[0]=='b' && ligne[1]=='y' && ligne[2]=='e'){
                fprintf(stdout,"Bonne journée\n");
                exit(0);
            }
        }
        int i=0;
        char c;
        c=ligne[i];
        int cpt = 0;
        while ((c != ' ') && c != '\0' && c != '\n'){
            req[i]=c;
            i++;
            cpt++;
            c = ligne[i];
        }

        if (strcmp(req,"get")==0 ){
            requete.type = GET;
            i++;//on passe l'espace
            cpt++;
            c = ligne[i];
            while (c != '\n'){
                requete.fic[i-cpt]=c;
                i++;
                c = ligne[i];
            }
            
            fd = open(requete.fic, O_RDWR, 0644); 
            if (fd==-1){ 
                fd = open(requete.fic, O_CREAT | O_RDWR, 0644); 
                if (fd<0){
                    perror("open");
                    exit(1);
                }
                offset = 0;
                requete.numero = 0;
                requete.index = 0;
            }
            else{ 
                offset = lseek(fd, 0, SEEK_END);
                requete.numero = offset / TAILLESOCKET;
                requete.index = offset % TAILLESOCKET;
            }
            Close(fd);
        }
        else if (strcmp(req,"put")==0 ){
            requete.type = PUT;
        }
        else if (strcmp(req,"ls")==0 ){
            requete.type = LS;
        }
        else { 
            requete.type = INCONNU;
            printf("TYPE DE REQUETE INCONNUE, PEUT ETRE A CHANGER\n");
        }

        Rio_writen(clientfd, &requete, sizeof(requete));
        Rio_readnb(&rio_client, &reponse, sizeof(reponse));
        sprintf(chemin, "./DATA/%s", requete.fic);
        
        switch(reponse.type){
            case SUCCES:
                fd = open(requete.fic, O_RDWR, 0644); 
                flag = 1;
                while (flag) {
                    if (Rio_readnb(&rio_client, &reponse, sizeof(reponse)) <= 0) break;
                    Rio_readnb(&rio_client, buf, reponse.taille);
                    
                    lseek(fd, reponse.numero * TAILLESOCKET, SEEK_SET);
                    Rio_writen(fd, buf, reponse.taille);
                    
                    fprintf(stdout,"Le paquet numéro %d/%d a bien été recu\n",reponse.numero+1,reponse.total);
                    
                    if (reponse.numero >= reponse.total - 1){
                        flag = 0;
                    }
                }
                fprintf(stdout,"Le fichier %s a bien été copié\n", requete.fic);
                Close(fd);
                rename(requete.fic, chemin);
                break;

            case FICHIER_INEXISTANT:
                fprintf(stdout,"Le fichier recherché n'a pas été trouvé\n");
                break;
            case ERREUR_REQ:
                fprintf(stdout,"La requete n'est pas valide, %s n'est pas une commande valide \n",req);
                break;
            default:
                fprintf(stdout,"autre\n");
                break;
        }
        memset(req, 0, sizeof(req));
        fprintf(stdout,"ftp>");
    }
    Close(clientfd);
    exit(0);
}