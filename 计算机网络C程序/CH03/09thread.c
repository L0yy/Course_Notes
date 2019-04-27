#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static int thread_exit_sigval = 0;

static char *
current_time_str ()
{
    static char timebuf[64] = { '\0' };
    time_t t;
    char *tmstr, *p;

    t = time (NULL);
    tmstr = ctime (&t);

    strcpy (timebuf, tmstr);
    p = strchr (timebuf, '\n');
    if ( p != NULL )
        *p = '\0';

    return timebuf;
}

static void *
thread_main (void *arg)
{
    while ( !thread_exit_sigval ) {
        printf ("[%s] [S] Thread tick!\n", current_time_str ());
        sleep (2);
    }

    printf ("[%s] [S] Thread exit NOW!\n", current_time_str ());
    thread_exit_sigval = 0;
    pthread_exit (NULL);
    return NULL;
}

int main ()
{
    pthread_t subthd;
    int ret;

    printf ("[%s] [M] Create sub-thread.\n", current_time_str ());
    ret = pthread_create (&subthd, NULL, thread_main, NULL);
    printf ("[%s] [M] Sub-thread created.\n", current_time_str ());

    sleep (7);

    printf ("[%s] [M] Send exit signal to sub-thread.\n",
            current_time_str ());
    thread_exit_sigval = 1;

    pthread_join (subthd, NULL);
    printf ("[%s] [M] Sub-thread exited!\n", current_time_str ());

    return 0;
}
