/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NB_PROC 3

pid_t pid[NB_PROC];
int pid_p; 

void cat(int connfd);

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */

void handler_child(int signal){
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler_int(int signal) {
    if (getpid() == pid_p) {
        for (int i = 0; i < NB_PROC; i++) {
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
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    

    port = 2121; //port par défaut
    
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);

    pid_p = getpid();

    fprintf(stdout,"Demarage du serveur sur le port n°%d",port);

    //boucle de fork pour connecter plusieurs clients sur le serveur 
    for( int i = 0; i < NB_PROC && ((pid[i] = Fork()) != 0);i++){
    }

    while (1) {//attente du serveur

        if ( pid_p != getpid() ){
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen,
                        client_hostname, MAX_NAME_LEN, 0, 0, 0);
            
            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                    INET_ADDRSTRLEN);
            
            printf("server connected to %s (%s)\n", client_hostname,
                client_ip_string);
                    
            cat(connfd);
            Close(connfd);
        }
    }
    exit(0);
}