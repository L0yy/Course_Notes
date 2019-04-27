#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main (int argc, char *argv[])
{
    int sock;
    struct sockaddr_in addr;

    if ( argc <= 2 ) {
        printf ("Usage: %s <ipaddr> <message>\n");
        return 1;
    }

    sock = socket (AF_INET, SOCK_DGRAM, 0);
    if ( sock == -1 ) {
        printf ("ERROR: Cannot make socket.\n");
        return 1;
    }

    memset (&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (7889);
    inet_aton (argv[1], &addr.sin_addr);

    sendto (sock, argv[2], strlen (argv[2]) + 1, 0,
            (struct sockaddr *)&addr,  sizeof (struct sockaddr_in));

    close (sock);
    return 0;
}
