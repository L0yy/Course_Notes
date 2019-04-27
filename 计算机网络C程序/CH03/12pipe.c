#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

static int pipefd[2];

static void parent_proc ()
{
    char *mesg = "Hello World!";

    close (pipefd[0]);
    write (pipefd[1], mesg, strlen (mesg) + 1);

    printf ("[Parent] Write Message to Pipe: %s\n", mesg);
}

static void child_proc ()
{
    char msgbuf[128] = { '\0' };

    close (pipefd[1]);
    read (pipefd[0], msgbuf, 127);
    printf ("[Child] Read Message from Pipe: %s\n", msgbuf);
}

int main ()
{
    int pid;

    pipe (pipefd);
    
    pid = fork ();
    if ( pid < 0 ) {
        printf ("ERROR: fork failed!\n");
        exit (1);
    }

    if ( pid != 0 )
        parent_proc ();
    else
        child_proc ();
    return 0;
}
