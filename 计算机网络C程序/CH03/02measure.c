#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

static void
timepass_usec (char *str)
{
    struct timeval tv1, tv2;
    struct timeval tvdiff;

    printf ("gettimeofday:\n");

    gettimeofday (&tv1, NULL);
    printf ("%s\n", str);
    gettimeofday (&tv2, NULL);

    printf ("Start:  %ld.%06ld s\n", tv1.tv_sec, tv1.tv_usec);
    printf ("End:    %ld.%06ld s\n", tv2.tv_sec, tv2.tv_usec);

    if (tv2.tv_usec < tv1.tv_usec) {
        tvdiff.tv_sec = tv2.tv_sec - tv1.tv_sec - 1;
        tvdiff.tv_usec = tv2.tv_usec + 1000000L - tv1.tv_usec;
    } else {
        tvdiff.tv_sec = tv2.tv_sec - tv1.tv_sec;
        tvdiff.tv_usec = tv2.tv_usec - tv1.tv_usec;
    }
    printf ("Passed: %ld.%06ld s\n", tvdiff.tv_sec, tvdiff.tv_usec);
}

static void
timepass_nsec (char *str)
{
    struct timespec tp1, tp2;
    struct timespec tpdiff;

    printf ("clock_gettime:\n");

    clock_gettime (CLOCK_MONOTONIC, &tp1);
    printf ("%s\n", str);
    clock_gettime (CLOCK_MONOTONIC, &tp2);

    printf ("Start:  %ld.%09ld s\n", (long)tp1.tv_sec, tp1.tv_nsec);
    printf ("End:    %ld.%09ld s\n", (long)tp2.tv_sec, tp2.tv_nsec);

    if (tp2.tv_nsec < tp1.tv_nsec) {
        tpdiff.tv_sec = tp2.tv_sec - tp1.tv_sec - 1;
        tpdiff.tv_nsec = tp2.tv_nsec + 1000000000L - tp1.tv_nsec;
    } else {
        tpdiff.tv_sec = tp2.tv_sec - tp1.tv_sec;
        tpdiff.tv_nsec = tp2.tv_nsec - tp1.tv_nsec;
    }
    printf ("Passed: %ld.%09ld s\n", tpdiff.tv_sec, tpdiff.tv_nsec);
}

int
main (int argc, char *argv[])
{
    char *str = "Hello World!";

    if ( argc > 1 )
        str = argv[1];

    timepass_usec (str);
    printf ("\n");

    timepass_nsec (str);

    return 0;
}
