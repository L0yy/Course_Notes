#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
proc_main ()
{
    printf ("Daemon started!\n");
    printf ("Quit after 15s...\n");
    sleep (15);
    printf ("Daemon is quiting NOW!\n");
}

static int
make_daemon ()
{
    pid_t pid;
    pid = fork ();
    if ( pid < 0 ) {
        fprintf (stderr, "ERROR in fork()\n");
        return -1;
    }

    /* Parent Process, Exit */
    if ( pid != 0 )
        exit (0);

    /* Unbound from current session */
    /* Become session leader and get PID */
    pid = setsid ();
    if ( pid < 0 ) {
        fprintf (stderr, "ERROR in setsid()\n");
        return -1;
    }

    return 0;
}

int main ()
{
    int ret = 0;

    ret = make_daemon ();
    if ( ret != 0 )
        return 0;

    proc_main ();
    return 0;
}
