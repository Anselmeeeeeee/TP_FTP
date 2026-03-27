/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NB_SLAVES 3 

int port_esclave[NB_SLAVES] = {2122, 2123, 2124};
int esclave_actuel = 0;

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
    

    clientlen = (socklen_t)sizeof(clientaddr);

    pid_p = getpid();


    //boucle de fork pour connecter plusieurs clients sur le serveur 
    for( int i = 0; i < NB_SLAVES;i++){
        if ((pid[i] = Fork()) == 0){
            int listenfd_esclave = Open_listenfd(port_esclave[i]);
            printf("[Esclave %d] Écoute sur le port %d\n", i, port_esclave[i]);
            while(1){
                int connfd_esclave = Accept(listenfd_esclave, (SA *)&clientaddr, &clientlen);
                while(cat(connfd_esclave) > 0){
                }
                Close(connfd_esclave);
            }
            exit(0);
        }
    }

    listenfd = Open_listenfd(2121);

    while (1) {//attente du serveur
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        int port = port_esclave[esclave_actuel];

        Rio_writen(connfd,&port,sizeof(int));
        esclave_actuel = ( esclave_actuel + 1 ) % NB_SLAVES;
        Close(connfd);
    }
    
    exit(0);
}