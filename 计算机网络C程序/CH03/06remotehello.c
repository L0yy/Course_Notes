#include <stdio.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
    int origfd;

    if ( argc > 1 ) {
        origfd = creat (argv[1], 0200);
        close (1);
        dup2 (origfd, 1);
    }

    printf ("Hello World!\n");
    return 0;
}
