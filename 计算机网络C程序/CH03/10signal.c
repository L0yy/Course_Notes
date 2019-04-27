#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>

static void
signal_handler (int signo)
{
    size_t btsz;
    void *bt[256];
    char **btstr;
    int i;

    printf ("Signal received: %d\n", signo);

    btsz = backtrace (bt, 256);
    btstr = backtrace_symbols (bt, btsz);

    printf ("Backtrace Stack:\n");
    for ( i = 0; i < btsz; i++ ) {
        printf ("%s\n", btstr[i]);
    }

    printf ("\nExit Now!\n");
    exit (0);
}

int main ()
{
    int i;

    signal (SIGHUP, signal_handler);
    signal (SIGINT, signal_handler);
    signal (SIGQUIT, signal_handler);
    signal (SIGILL, signal_handler);
    signal (SIGABRT, signal_handler);
    signal (SIGBUS, signal_handler);
    signal (SIGFPE, signal_handler);
    signal (SIGUSR1, SIG_IGN);
    signal (SIGSEGV, signal_handler);
    signal (SIGUSR2, SIG_IGN);
    signal (SIGPIPE, SIG_IGN);
    signal (SIGALRM, signal_handler);
    signal (SIGTERM, signal_handler);
    signal (SIGSTKFLT, signal_handler);
    signal (SIGVTALRM, signal_handler);
    signal (SIGPROF, signal_handler);
    signal (SIGIO, signal_handler);
    signal (SIGPWR, signal_handler);
    for ( i = 34; i <= 64; i++ )
        signal (i, SIG_IGN);

    printf ("Start...\n");
    while ( 1 ) {
        sleep (10);
    }

    return 0;
}
