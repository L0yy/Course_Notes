#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

int
main (int argc, char *argv[])
{
    int i;
    struct stat buf;
    char *modestr = "X";

    for ( i = 1; i < argc; i++ ) {
        printf ("%s: ", argv[i]);
        if ( lstat (argv[i], &buf) < 0 ) {
            printf ("** error **\n");
            continue;
        }

        if ( S_ISREG (buf.st_mode) )
            modestr = "regular";
        else if ( S_ISDIR (buf.st_mode) )
            modestr = "directory";
        else if ( S_ISCHR (buf.st_mode) )
            modestr = "character special";
        else if ( S_ISBLK (buf.st_mode) )
            modestr = "block special";
        else if ( S_ISFIFO (buf.st_mode) )
            modestr = "fifo";
        else if ( S_ISLNK (buf.st_mode) )
            modestr = "symbolic link";
        else if ( S_ISSOCK (buf.st_mode) )
            modestr = "socket";
        else
            modestr = "** unknown mode **";

        printf ("%s\n", modestr);
    }

    return 0;
}
