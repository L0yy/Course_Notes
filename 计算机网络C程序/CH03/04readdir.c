#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char **argv)
{
    const char *pathname = ".";
    DIR *dir;
    struct dirent *ptr;
    int i = 0;

    if ( argc > 1 )
        pathname = argv[1];

    dir = opendir (pathname);
    if ( dir == NULL ) {
        printf ("Not a direction.\n");
        exit(0);
    }

    printf ("Files in %s:\n", pathname);
    while ( (ptr = readdir(dir)) != NULL ) {
        i++;
        printf ("%5d [0x%04x]: %s\n", i, ptr->d_type, ptr->d_name);
    }
    closedir(dir);

    printf ("Total: %d\n", i);
    return 0;
}
