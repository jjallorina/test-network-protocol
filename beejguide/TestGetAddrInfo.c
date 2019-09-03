
/*
** showip.c -- show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int main( int argc, char **argv)
{
    int s = 0;
    int status = 0;
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof(struct addrinfo));

    if((status = getaddrinfo("www.codeontechnologies.com", "http", &hints, &res)
    < 0))
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    } 
    return 0;

}
