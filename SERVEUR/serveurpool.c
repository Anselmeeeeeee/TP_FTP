#include "csapp.h"
#include "structure.h"

#define MAX_NAME_LEN 256
#define NB_SLAVES 10

//initialisation de variables
esclave_t tab_esclave[NB_SLAVES];
pid_t pid[NB_SLAVES];
int pid_p; 

int traitement(int connfd);

void handler_child(int signal){ //gère l'élimination des fils
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler_int(int signal) {
    //arrête proprement le serveur et ses esclaves
    if (getpid() == pid_p) {
        for (int i = 0; i < NB_SLAVES; i++) {
            kill(pid[i], SIGINT);
        }
    }
    exit(0);
}

int main(int argc, char **argv)
{   
    //redirection des signaux
    Signal(SIGINT,handler_int);
    Signal(SIGCHLD, handler_child); 

    //initialisation de variables
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int port = 2122;
    clientlen = (socklen_t)sizeof(clientaddr);
    pid_p = getpid();


    //initialisation du serveur
    for( int i = 0; i < NB_SLAVES;i++){
        tab_esclave[i].dispo = OUI; //mets la disponibilités de tout les esclaves à OUI
        tab_esclave[i].port = 2122 + i; //Attribution du port aux esclaves
        if ((pid[i] = Fork()) == 0){ //esclaves
            //connection maitre esclave 
            int listen_ctrl = Open_listenfd(tab_esclave[i].port + 100); 
            int socket_maitre = Accept(listen_ctrl, NULL, NULL); 
            printf("[Esclave %d] Interconnecté au Maître sur le socket %d\n", i, socket_maitre);

            //ouverture des esclaves aux clients
            int listenfd_esclave = Open_listenfd(tab_esclave[i].port);
            printf("[Esclave %d] Écoute sur le port %d\n", i, tab_esclave[i].port);

            while(1){ //les esclaves ne finissent jamais 
                
                int connfd_esclave = Accept(listenfd_esclave, (SA *)&clientaddr, &clientlen);//attente d'un client
                while(traitement(connfd_esclave) > 0){//traitement des commandes envoyées par le client
                }
                Close(connfd_esclave);//fin de la communication avec le client

                
                //signale au maître sa disponibilité pour acceuillir un client
                disponibilite signal_dispo = OUI;
                Rio_writen(socket_maitre, &signal_dispo, sizeof(disponibilite));
            }
            exit(0);
        }
        else{//maitre
            usleep(100000); //pour pas que le maitre n'aille trop vite, attente d'un esclave
            //conction du maître à l'esclave
            tab_esclave[i].sockfd = Open_clientfd("localhost", tab_esclave[i].port + 100);
            printf("[Maître] Interconnexion avec l'esclave %d réussie.\n", i);
        }
    }

    listenfd = Open_listenfd(2121);//port du maître
    printf("[Maître] En attente de clients sur le port 2121...\n");
    while (1) {//attente du serveur
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        int i;
        int index = -1;
        for (i = 0; i<NB_SLAVES;i++){ //recherche un esclave libre pour répondre au client
            if ( tab_esclave[i].dispo == OUI){
                port = tab_esclave[i].port;
                tab_esclave[i].dispo = NON; //l'esclave a été selectionné, il n'est plus disponible
                index = i;
                break; //on sort de la boucle lorsque l'esclave numéro index est libre
            }
            
        }
        if (index != -1 ){ //si un esclave est libre, on le connecte au client
            Rio_writen(connfd, &port, sizeof(int));
            printf("[Maître] Client redirigé vers l'esclave %d (Port %d)\n", index, port);
        } else { //aucun esclave libre n'a été trouvé 
            // on avertit le client que tous les ports sont occupés donc qu'il ne peut pas se connecter
            int error_val = -1;
            Rio_writen(connfd, &error_val, sizeof(int));
            printf("[Maître] Tous les esclaves sont occupés !\n");
        }
        Close(connfd);
    }
    exit(0);
}