#include <stdio.h>
#include <string.h>

static int
proc_add (int argc, char *argv[])
{
    int op1, op2, sum;

    if ( argc <= 2 ) {
        printf ("ERROR: Two parameters needed.\n");
        return 1;
    }

    op1 = atoi (argv[1]);
    op2 = atoi (argv[2]);
    sum = op1 + op2;

    printf ("%d + %d = %d\n", op1, op2, sum);
    return 0;
}

static int
proc_factorial (int argc, char *argv[])
{
    int base, fact = 1;
    int i;

    if ( argc <= 1 ) {
        printf ("ERROR: One parameter needed.\n");
        return 1;
    }

    base = atoi (argv[1]);
    for ( i = 1; i <= base; i++ ) {
        fact *= i;
    }

    printf ("%d! = %d\n", base, fact);
    return 0;
}

static int
proc_hello (int argc, char *argv[])
{
    printf ("Hello World!\n");
    return 0;
}

int
main (int argc, char *argv[])
{
    int ret;

    if ( argc <= 1 ) {
        printf ("Use sub-command!\n");
        return 1;
    }

    if ( strcmp (argv[1], "add") == 0 ) {
        ret = proc_add (argc - 1, argv + 1);
    } else if ( strcmp (argv[1], "factorial") == 0 ) {
        ret = proc_factorial (argc - 1, argv + 1);
    } else if ( strcmp (argv[1], "hello") == 0 ) {
        ret = proc_hello (argc - 1, argv + 1);
    } else {
        printf ("ERROR: Unknown sub-command!\n");
        return 2;
    }

    return 0;
}
