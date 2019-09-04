#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT "4000"
#define BACKLOG 10



void *get_inaddr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in *)sa)->sin_addr);
    else
        return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
    int status = 0;
    int rv = 0;
    int sockfd = 0;
    int newfd = 0;
    struct addrinfo hints, *servinfo = NULL, *p = NULL;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    if((rv = getaddrinfo( NULL, PORT, &hints, &servinfo)) < 0)
    {
        fprintf(stderr, "getaddrinfo [%s]\n", gai_strerror(rv));
        exit(1);
    } 

    for( p = servinfo; p != NULL; p = p->ai_next )
    {
        // call socket to get a socket file descriptor
       if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
       {
            perror("server socket:");
            continue;
       }
        //setsockopt to reuse the port
        if((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) <
        0 ) {
            perror("server setsockopt:");
            exit(1);
        }
        //bind socket call
        if( bind(sockfd, p->ai_addr, p->ai_addrlen) < 0)
        {
            perror("server bind:");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);
   /* not necessarily frees on heap since it has a value after calling
    freeaddrinfo() */ 

//#define SERVINFO
#if defined(SERVINFO)
    printf("%d\n%d\n%d\n%d\n",servinfo->ai_flags, servinfo->ai_family, servinfo->ai_socktype,
    servinfo->ai_protocol );  
#endif
    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
   
    if(listen(sockfd, BACKLOG) < 0)
    {
        perror("server listen:");
        exit(1);
    }
  
#define BUFSIZE 10000
    ssize_t nread = 0;
    ssize_t nsent = 0;
    char buffer[BUFSIZE] = {0};
    char respmsg[] = "I got your message\n";
    //accept connections from incoming request ei. external parties
    while(1){
    sin_size = sizeof(their_addr);
        if((newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) < 0)
        {
            perror("server accept:");
            continue;
        }
       
        inet_ntop(their_addr.ss_family, get_inaddr((struct sockaddr *)&their_addr), s, sizeof(s));         

        printf("server running: got connection from [%s]\n", s);
        if((nread = recv(newfd, buffer, BUFSIZE, 0)) < 0)
        {
            perror("server recv:");
            continue;
        }

        printf("request message received [%ld bytes]\n", nread);
        printf("request message \n [%s]\n", buffer);
    
        if((nsent = send(newfd , respmsg, strlen(respmsg), 0)) < 0)
        {
            perror("server send:");
            close(newfd);   //close client socket fd
            close(sockfd);  //close server socket fd
            exit(1);
        } 
        else{
            printf("sending response message [%ld bytes]\n", nsent);
            printf("response message sent \n[%s]\n", respmsg);
        }
    }
    close(sockfd);  //close server socket fd

    return 0;
}
