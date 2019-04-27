#include <stdio.h>
#include <time.h>

static void
dump_time (struct tm *tm)
{
    printf ("%d-%d-%d %d:%d:%d (%d/7, %d/365, dst=%d)",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec,
            tm->tm_wday, tm->tm_yday, tm->tm_isdst);
}

int main ()
{
    time_t curtime;
    struct tm *localtm, *gmtm;
    char *ctmstr, *localstr, *gmstr;

    curtime = time (NULL);
    ctmstr = ctime (&curtime);
    printf ("CTIME: %s\n", ctmstr);

    localtm = localtime (&curtime);
    printf ("Local Time: ");
    dump_time (localtm);
    printf ("\n");

    localstr = asctime (localtm);
    printf ("[ASC TIME]: %s\n", localstr);

    gmtm = gmtime (&curtime);
    printf ("GM Time: ");
    dump_time (gmtm);
    printf ("\n");

    gmstr = asctime (gmtm);
    printf ("[ASC TIME]: %s\n", gmstr);

    return 0;
}
