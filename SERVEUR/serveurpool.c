/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"
#include "structure.h"

#define MAX_NAME_LEN 256
#define NB_SLAVES 3 

esclave_t tab_esclave[NB_SLAVES];

//int esclave_actuel = 0;

pid_t pid[NB_SLAVES];
int pid_p; 

int cat(int connfd);

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */

void handler_child(int signal){
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler_int(int signal) {
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

    //initialisation de vairables
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int port = 2122;
    clientlen = (socklen_t)sizeof(clientaddr);

    pid_p = getpid();


    //boucle de fork pour connecter plusieurs clients sur le serveur 
    for( int i = 0; i < NB_SLAVES;i++){
        tab_esclave[i].dispo = OUI;
        tab_esclave[i].port = 2122 + i;
        if ((pid[i] = Fork()) == 0){ //esclaves
            int listen_ctrl = Open_listenfd(tab_esclave[i].port + 100); //connection maitre esclave 
            int socket_maitre = Accept(listen_ctrl, NULL, NULL);
            printf("[Esclave %d] Interconnecté au Maître sur le socket %d\n", i, socket_maitre);


            int listenfd_esclave = Open_listenfd(tab_esclave[i].port);
            printf("[Esclave %d] Écoute sur le port %d\n", i, tab_esclave[i].port);
            while(1){
                int connfd_esclave = Accept(listenfd_esclave, (SA *)&clientaddr, &clientlen);
                while(cat(connfd_esclave) > 0){
                }
                Close(connfd_esclave);
                disponibilite signal_dispo = OUI;
                Rio_writen(socket_maitre, &signal_dispo, sizeof(disponibilite));
            }
            exit(0);
        }
        else{//maitre
            usleep(100000); //pour pas que le maitre n'aille trop vite
            tab_esclave[i].sockfd = Open_clientfd("localhost", tab_esclave[i].port + 100);
            printf("[Maître] Interconnexion avec l'esclave %d réussie.\n", i);
        }
    }

    listenfd = Open_listenfd(2121);
    printf("[Maître] En attente de clients sur le port 2121...\n");
    while (1) {//attente du serveur
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        int i;
        int index = -1;
        for (i = 0; i<NB_SLAVES;i++){
            if ( tab_esclave[i].dispo == OUI){
                port = tab_esclave[i].port;
                tab_esclave[i].dispo = NON;
                index = i;
                break;
                //on sort de la boucle lorsque l'esclave numéro index est libre
            }
            
        }
        if (index != -1 ){
            Rio_writen(connfd, &port, sizeof(int));
            printf("[Maître] Client redirigé vers l'esclave %d (Port %d)\n", index, port);
        } else {
            // on avertit l'esclave que tous les ports sont occupés en lui envoyant -1 en port de connection
            int error_val = -1;
            Rio_writen(connfd, &error_val, sizeof(int));
            printf("[Maître] Tous les esclaves sont occupés !\n");
        }
        Close(connfd);
    }
    exit(0);
}