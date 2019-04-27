#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static void *
alloc_shared_mem (key_t shmkey, int size, int *shmidx, int *newcreated)
{
    int _shmidx = -1;
    int _newcreated = 0;
    void *retptr = NULL;

    _shmidx = shmget (shmkey, size, 0666);
    if ( _shmidx < 0 ) {
        _shmidx = shmget (shmkey, size, 0666 | IPC_CREAT);
        if ( _shmidx < 0 )
            return NULL;

        _newcreated = 1;
    }

    retptr = shmat (_shmidx, NULL, 0);
    if ( retptr == NULL || retptr == (void *)-1 ) {
        if ( _newcreated )
            shmctl (_shmidx, IPC_RMID, 0);
        return NULL;
    }

    if ( shmidx != NULL )
        *shmidx = _shmidx;
    if ( newcreated != NULL )
        *newcreated = _newcreated;
    return retptr;
}

static void
free_shared_mem (int shmidx, void *ptr)
{
    if ( ptr != NULL )
        shmdt (ptr);
}

#define SHMEM_KEY_ID  0x1A73092A

static int
proc_shmem_get (int argc, char *argv[])
{
    char *shbuf;
    int shmidx, newcreated;

    shbuf = alloc_shared_mem (SHMEM_KEY_ID, 1024, &shmidx, &newcreated);
    if ( newcreated )
        memset (shbuf, 0, 1024);

    printf ("Data:\n");
    printf ("%s\n", shbuf);

    free_shared_mem (shmidx, shbuf);
    return 0;
}

static int
proc_shmem_set (int argc, char *argv[])
{
    char *shbuf, *arg = "";
    int shmidx, newcreated;

    shbuf = alloc_shared_mem (SHMEM_KEY_ID, 1024, &shmidx, &newcreated);
    if ( newcreated )
        memset (shbuf, 0, 1024);

    if ( argc > 1 )
        arg = argv[1];

    strncpy (shbuf, arg, 1023);

    free_shared_mem (shmidx, shbuf);
    return 0;
}

int main (int argc, char *argv[])
{
    if ( argc <= 1 ) {
        printf ("Usage: %s get|set <string>\n", argv[0]);
        return 0;
    }

    if ( strcmp (argv[1], "get") == 0 )
        return proc_shmem_get (argc - 1, argv + 1);
    else if ( strcmp (argv[1], "set") == 0 )
        return proc_shmem_set (argc - 1, argv + 1);
    else
        printf ("Usage: %s get|set <string>\n", argv[0]);

    return 0;
}
