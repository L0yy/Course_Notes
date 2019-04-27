#include <stdio.h>
#include <string.h>
#include <getopt.h>

static int jokeflag = 'a';

static const struct option long_opts[] = {
    { "string",      1, NULL,      's' },
    { "decimal",     1, NULL,      'd' },
    { "float",       1, NULL,      'f' },
    { "joke",        0, &jokeflag, 'j' },
    { "hello",       0, NULL,       0  },
    { 0, 0, 0, 0 }
};

static const char short_opts[] =
    "s:d:f:j";

int
main (int argc, char *argv[])
{
    int ret, optint;
    float optfloat;
    int chidx = -1, hasopt = 0;
    char c;

    printf ("Options:\n");
    while ( (c = getopt_long (argc, argv,
                              short_opts, long_opts, &chidx)) != -1 ) {
        switch ( c ) {
          case 's':
            printf (" - string [s]: %s\n", optarg);
            break;

          case 'd':
            ret = sscanf (optarg, "%d", &optint);
            if ( ret >= 1 )
                printf (" - decimal [d]: %d\n", optint);
            else
                printf (" - decimal [d]: NaN\n");
            break;

          case 'f':
            ret = sscanf (optarg, "%f", &optfloat);
            if ( ret >= 1 )
                printf (" - float [f]: %f\n", optfloat);
            else
                printf (" - float [f]: NaN\n");
            break;

          case 'j':
            printf (" - joke [j]: Hahaha, Flag=%d\n", jokeflag);
            break;

          case 0:
            if ( strcmp (long_opts[chidx].name, "hello") == 0 ) {
                printf (" - hello [\\0]: Hello World\n");
            } else if ( strcmp (long_opts[chidx].name, "joke") == 0 ) {
                printf (" - joke [j]: Haha, Flag=%d\n", jokeflag);
            } else {
                printf (" - Unknown option!\n");
            }
            break;

          default:
            printf (" - Unknown option!\n");
            break;
        }
        hasopt = 1;
    }

    if ( !hasopt )
        printf ("  [EMPTY]\n");
    return 0;
}
