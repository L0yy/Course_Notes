#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int
connect_to_server (struct in_addr *ipaddr)
{
    int sock, ret;
    struct sockaddr_in addr;

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if ( sock == -1 ) {
        printf ("ERROR: Cannot create server socket!\n");
        return -1;
    }

    printf ("Socket created successfully.\n");

    memset (&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (7799);
    memcpy (&addr.sin_addr, ipaddr, sizeof (struct in_addr));

    ret = connect (sock, (struct sockaddr *)&addr,
                   sizeof (struct sockaddr_in));
    if ( ret == -1 ) {
        printf ("ERROR: Cannot connect to server!\n");
        close (sock);
        return -1;
    }

    printf ("Connect to server successfully.\n");
    return sock;
}

int main (int argc, char *argv[])
{
    int sock, ret, i;
    char recvmsgbuf[1024];
    struct in_addr ipaddr;
    socklen_t recvlen;

    if ( argc <= 2 ) {
        printf ("Usage: %s <IPADDR> <MESSAGE> ...\n", argv[0]);
        return 1;
    }

    ret = inet_aton (argv[1], &ipaddr);
    if ( ret == 0 ) {
        printf ("Usage: %s <IPADDR> <MESSAGE> ...\n", argv[0]);
        return 1;
    }

    sock = connect_to_server (&ipaddr);
    if ( sock == -1 )
        return 1;

    for ( i = 2; i < argc; i++ ) {
        send (sock, argv[i], strlen (argv[i]) + 1, 0);
        printf ("[C>S] %s\n", argv[i]);

        recvlen = recv (sock, recvmsgbuf, 1023, 0);
        if ( recvlen <= 0 ) {
            printf ("Connected closed by server.\n");
            break;
        }

        printf ("[S>C] %s\n", recvmsgbuf);
    }

    close (sock);
    printf ("Connection closed.\n");
    return 0;
}
