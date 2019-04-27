#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int
get_server_socket ()
{
    int sock, ret;
    uint32_t flags;
    struct sockaddr_in addr;

    sock = socket (AF_INET, SOCK_DGRAM, 0);
    if ( sock < 0 ) {
        printf ("ERROR: Cannot make socket.\n");
        return -1;
    }

    printf ("Make socket successfully!\n");

    memset (&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (7889);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);

    ret = bind (sock, (struct sockaddr *)&addr,
                sizeof (struct sockaddr_in));
    if ( ret < 0 ) {
        printf ("ERROR: Cannot bind port.\n");
        close (sock);
        return -1;
    }

    printf ("Bind port successfully!\n");
    return sock;
}

int main (int argc, char *argv[])
{
    int sock, len;
    char buf[1024] = { '\0' };
    int retlen;
    struct sockaddr_in caddr;

    sock = get_server_socket ();
    if ( sock < 0 )
        return 1;

    memset (&caddr, 0, sizeof (struct sockaddr_in));

    while ( 1 ) {
        len = sizeof (struct sockaddr_in);
        retlen = recvfrom (sock, buf, 1024, 0,
                           (struct sockaddr *)&caddr, &len);
        if ( retlen < 0 ) {
            printf ("Receive from socket failed. [%d, %d:%s]\n",
                    retlen, errno, strerror (errno));
            break;
        }

        printf ("Receive data from %s:%d: [ %s ]\n",
                inet_ntoa (caddr.sin_addr), ntohs (caddr.sin_port), buf);

        if ( strcmp (buf, "quit") == 0 ) {
            printf ("Receive quit message.\n");
            break;
        }
    }

    close (sock);
    printf ("Socket closed.\n");
    return 0;
}
