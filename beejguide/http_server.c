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

/*
struct addrinfo
{
  int ai_flags;                 // Input flags.  
  int ai_family;                // Protocol family for socket.  
  int ai_socktype;              // Socket type.  
  int ai_protocol;              // Protocol for socket.  
  socklen_t ai_addrlen;         // Length of socket address.  
  struct sockaddr *ai_addr;     // Socket address for socket.  
  char *ai_canonname;           // Canonical name for service location.  
  struct addrinfo *ai_next;     //  Pointer to next in list. 
};

struct sockaddr_storage
{
    __SOCKADDR_COMMON (ss_);     // Address family, etc.  
    char __ss_padding[_SS_PADSIZE];
    __ss_aligntype __ss_align;   // Force desired alignment.  
};
*/

#define USE_HTTP_PORT
#if defined(USE_HTTP_PORT)
#define PORT "8080"
#else
#define PORT "4000"
#endif

#define BACKLOG 10

void sigchld_handler(int s)
{
    //waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;

}

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in *)sa)->sin_addr);
    
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
    int status = 0;
    int sockfd = 0;
    int newfd = 0;
    struct addrinfo hints, *servinfo = NULL, *p = NULL;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    ssize_t nread = 0;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    //hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo( NULL, PORT, &hints, &servinfo) != 0))
    {
        fprintf(stderr, "getaddrinfo: [%s]\n", gai_strerror(rv));
        return 1;
    }

    printf("preparing to call socket and bind syscall . . .\n");
    printf("bind using port [%s] \n", PORT);
    for( p = servinfo; p != NULL; p = p->ai_next)
    {
        if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
        servinfo->ai_protocol)) == -1 ) {
            perror("server: socket");
            continue;
        }
        else
            printf("successfuly executed socket syscall with sockfd[%d]\n", sockfd);

        if((setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, 
            sizeof(int))) == -1 ) {  
            perror("server: setsockopt");
            exit(1);
        }
        else
            printf("successfuly executed setsockopt syscall\n");


        if( bind( sockfd, p->ai_addr, p->ai_addrlen) == -1 ) {
            perror("server: bind");
            continue;
        }

        break;
    }
    
    printf("freeing servinfo . .\n");
    freeaddrinfo(servinfo);

    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1){
        perror("listen");
        exit(1);
    }

    //setup signal handler
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction:");
        exit(1);
    }

    printf("server running: waiting for connections . . .\n");
    
//    const char message[] = "This message sent to you . . .";
    char *http_message = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!"; 
    char buffer[30000] = {0};
    while(1){
        sin_size = sizeof(their_addr);
        if( (newfd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
        == -1 )
        {
            perror("accept:");
            continue;
        } 

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr
        *)&their_addr), s, sizeof(s)); 
        printf("server running: got connection from [%s]\n", s);
     
        if((nread = recv(newfd, buffer, sizeof(buffer), 0)) == -1)
        {
            perror("recv:");
            continue;
        }
        
        printf("reading request message received from client [%ld bytes received]\n",nread);
        printf("received message \n [%s]\n", buffer);

        if(send(newfd, http_message, strlen(http_message), 0) == -1)
        {
            perror("send:");
            close(newfd);
            close(sockfd);
            exit(1);
        }
        else
            printf("sending response message to client \n [%s] \n", http_message);  
        
    }
    
    close(sockfd);
    return 0;
    
}
