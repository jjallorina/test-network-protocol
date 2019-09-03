#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int status = 0;
    int sockfd = 0;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if( status = getaddrinfo( NULL, "4000", &hints, &res) < 0)
        printf("status err [%04d] . . \n", __LINE__);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    printf("sockfd [%d] \n", sockfd);

    freeaddrinfo(res);

    if( status = getaddrinfo( NULL, "4001", &hints, &res) < 0)
        printf("status err [%04d] . . \n", __LINE__);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    printf("sockfd [%d] \n", sockfd);
    
    freeaddrinfo(res);

    if( status = getaddrinfo( NULL, "4002", &hints, &res) < 0)
        printf("status err [%04d] . . \n", __LINE__);


    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   
    printf("sockfd [%d] \n", sockfd);

    freeaddrinfo(res);

    return 0;

}
