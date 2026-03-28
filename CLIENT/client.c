/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include "structure.h"

int main(int argc, char **argv)
{   
    //initialisations de variables
    int clientfd, port_maitre = 2121, port_esclave; //port du serveur maitre par défaut 2121
    char *host, buf[MAXLINE];
    char chemin[MAXLINE+10]; //stockage dans un sous dossier DATA, donc nécessité de mettre le nom du sous dossier 
    request_t requete;
    response_t reponse;
    char req[TAILLENOM];
    rio_t rio_client ;
    rio_t rio_maitre;
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

    //connection au serveur
    clientfd = Open_clientfd(host, port_maitre);
    Rio_readinitb(&rio_maitre, clientfd);
    
    //connection au maitre pour récupérer port esclave libre
    if (Rio_readnb(&rio_maitre, &port_esclave, sizeof(int)) <= 0) {
        fprintf(stderr, "Impossible de récupérer le port esclave\n");
        exit(1);
    }
    Close(clientfd); //déconnection du maitre 
    
    printf("Redirection par le maître vers le port : %d\n", port_esclave);
    //Connection à un esclave depuis son port
    clientfd = Open_clientfd(host, port_esclave);

    Rio_readinitb(&rio_client, clientfd);
    printf("Connecté à l'esclave sur le port %d\n", port_esclave);
    fprintf(stdout,"ftp>");

    //récupère la commande à envoyer au serveur 
    while(fgets(ligne,MAXLINE,stdin)){
        memset(&requete,0,sizeof(requete));
        
        //gestion de la commande bye en dur 
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
        while ((c != ' ') && c != '\0' && c != '\n'){//récupération de la requete
            req[i]=c;
            i++;
            cpt++;
            c = ligne[i];
        }
        
        //gestion de la commande get et de son fichier
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
            
            //ouverture du fichier temporaire non complet s'il existe 
            fd = open(requete.fic, O_RDWR, 0644); 
            if (fd==-1){ 
                //il n'existe pas, création du fichier en question
                fd = open(requete.fic, O_CREAT | O_RDWR, 0644); 
                if (fd<0){ //échec de la création
                    perror("open");
                    exit(1);
                }
                offset = 0;
                requete.numero = 0;
                requete.index = 0;
            }
            else{ //recupération de l'offset si le téléchargement du fichier avait été interrompu
                offset = lseek(fd, 0, SEEK_END);
                requete.numero = offset / TAILLESOCKET;
                requete.index = offset % TAILLESOCKET;
            }
            Close(fd);
        }
        else if (strcmp(req,"put")==0 ){//gestion de la commande put 
            requete.type = PUT;
        }
        else if (strcmp(req,"ls")==0 ){//gestion de la commande ls
            requete.type = LS;
            int j = 0; 
            int debut = strlen(req) + 1; 
            int fin = strlen(ligne) - 1; 
            for (int i = debut; i < fin; i++) { //gestion des options de la commande ls 
                requete.fic[j] = ligne[i];
                j++;
            }
            requete.fic[j] = '\0'; 
        }
        else { //Si la commande n'existe pas 
            requete.type = INCONNU;
            printf("TYPE DE REQUETE INCONNUE\n");
        }

        Rio_writen(clientfd, &requete, sizeof(requete)); //Envoye de la requête au serveur
        Rio_readnb(&rio_client, &reponse, sizeof(reponse)); //récupération de la réponse du serveur
        sprintf(chemin, "./DATA/%s", requete.fic); //chemin pour la destination finale du fichier transféré
        
        switch(reponse.type){//gestion de la réponse
            case SUCCES: //le serveur a effectué avec succès la requête demandée
                if ( requete.type == GET){ //Traitement du résultat d'une requête GET
                    fd = open(requete.fic, O_RDWR, 0644); //le fiichier existe forcemment, on l'a créé plus tôt
                    flag = 1; //indique si on a recu tous les paquets 1 = non, 0 = oui
                    while (flag) {
                        if (Rio_readnb(&rio_client, &reponse, sizeof(reponse)) <= 0) break; //Récupère l'en-tête d'un paquet
                        Rio_readnb(&rio_client, buf, reponse.taille); // récupère le contenu d'un paquet
                        
                        lseek(fd, reponse.numero * TAILLESOCKET, SEEK_SET); //on se positionne au bon emplacement pour recevoir le paquet en question
                        Rio_writen(fd, buf, reponse.taille); //écriture du paquet dans le fichier
                        
                        fprintf(stdout,"Le paquet numéro %d/%d a bien été recu\n",reponse.numero+1,reponse.total);
                        
                        if (reponse.numero >= reponse.total - 1){//si on a recu le dernier paquet
                            flag = 0;
                        }
                    }
                    //fichier entièrement copié
                    fprintf(stdout,"Le fichier %s a bien été copié\n", requete.fic);
                    Close(fd);
                    rename(requete.fic, chemin); //Déplace le fichier à sa destination finale, dans le dossier DATA
                    memset(buf, 0, sizeof(buf)); //Vide le buffer
                }
                else if (requete.type == LS){  //Traitement du résultat d'une requête LS
                    if(Rio_readnb(&rio_client, &reponse, sizeof(reponse)) <= 0) return 0; //en-tête
                    Rio_readnb(&rio_client, buf, reponse.taille); //corps de la réponse

                    fprintf(stdout,"%s\n",buf);
                    memset(buf, 0, sizeof(buf)); 
                }
                break;

            case FICHIER_INEXISTANT: //Le fichier n'a pas été trouvé
                fprintf(stdout,"Le fichier recherché n'a pas été trouvé\n");
                remove(requete.fic); //suppression du fichier temporaire inutile créé
                break;
            case ERREUR_REQ: //La requête est invalide
                fprintf(stdout,"La requete n'est pas valide, %s n'est pas une commande valide \n",req);
                break;
            default:
                fprintf(stdout,"autre\n");
                break;
        }
        memset(req, 0, sizeof(req)); //On vide la requête
        fprintf(stdout,"ftp>");
    }
    Close(clientfd);
    exit(0);
}