#include <stdio.h>
#include <string.h>

static char cmdbuf[1024] = { '\0' };
static char lnbuf[1024] = { '\0' };

int main (int argc, char *argv[])
{
    FILE *fp;

    if ( argc <= 1 )
        snprintf (cmdbuf, 1023, "ls");
    else
        snprintf (cmdbuf, 1023, "ls %s", argv[1]);

    fp = popen (cmdbuf, "r");

    while ( fgets (lnbuf, 1024, fp) != NULL ) {
        char *lnch = strchr (lnbuf, '\n');
        if ( lnch != NULL )
            *lnch = '\0';

        printf ("%s\n", lnbuf);
    }

    pclose (fp);
    return 0;
}
