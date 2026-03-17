/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"


void cat(int connfd)
{
    char buf[MAXLINE];
    rio_t rio_client, rio_file;
    int fd;
    ssize_t n;

    Rio_readinitb(&rio_client, connfd);
    

    if (Rio_readlineb(&rio_client, buf, MAXLINE) <= 0) return;

    buf[strlen(buf)-1] = '\0'; 

    if ((fd = open(buf, O_RDONLY, 0)) < 0) {
        char *msg = "Impossible d'ouvrir le fichier\n";
        Rio_writen(connfd, msg, strlen(msg));
        return;
    }

    Rio_readinitb(&rio_file, fd);
    while ((n = Rio_readlineb(&rio_file, buf, MAXLINE)) > 0) {
        Rio_writen(connfd, buf, n); 
    }
    
    Close(fd);
}

