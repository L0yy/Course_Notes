#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int
get_server_socket ()
{
    int sock, ret;
    struct sockaddr_in addr;

    sock = socket (PF_INET, SOCK_STREAM, 0);
    if ( sock == -1 ) {
        printf ("ERROR: Cannot create server socket!\n");
        return -1;
    }

    printf ("Socket created successfully.\n");

    memset (&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons (7799);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);

    ret = bind (sock, (struct sockaddr *)&addr, sizeof (struct sockaddr_in));
    if ( ret < 0 ) {
        printf ("ERROR: Cannot bind port to server socket!\n");
        close (sock);
        return -1;
    }

    printf ("Binding port to server socket successfully!\n");
    return sock;
}

static int
connect_with_client (int svrsock)
{
    int commsock, ret;
    struct sockaddr_in caddr;
    socklen_t addrlen;

    ret = listen (svrsock, 1);
    if ( ret == -1 ) {
        printf ("ERROR: Cannot wait to connected.\n");
        return -1;
    }

    printf ("Waiting to connected...\n");

    addrlen = sizeof (struct sockaddr_in);
    commsock = accept (svrsock, (struct sockaddr *)&caddr, &addrlen);
    if ( commsock == -1 ) {
        printf ("ERROR: Connect with client failed!\n");
        return -1;
    }

    printf ("Connected with client %s:%d\n",
            inet_ntoa (caddr.sin_addr), ntohs (caddr.sin_port));

    return commsock;
}

int main ()
{
    int svrsock, commsock;
    char recvmsgbuf[1024] = { '\0' };
    char *sentmsg = "Hello from server!";
    socklen_t recvlen;

    svrsock = get_server_socket ();
    if ( svrsock == -1 )
        return 1;

    commsock = connect_with_client (svrsock);
    if ( commsock == -1 ) {
        close (svrsock);
        return 1;
    }

    while ( 1 ) {
        recvlen = recv (commsock, recvmsgbuf, 1023, 0);
        if ( recvlen <= 0 ) {
            printf ("Communication finished!\n");
            break;
        }
        printf ("[C>S] %s\n", recvmsgbuf);

        send (commsock, sentmsg, strlen (sentmsg) + 1, 0);
        printf ("[S>C] %s\n", sentmsg);
    }

    close (commsock);
    close (svrsock);
    return 0;
}
