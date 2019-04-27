#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/sem.h>

static int sem_create (int value);
static int sem_delete (int semid);
static int sem_set_value (int semid, int value);
static int sem_p (int semid);
static int sem_v (int semid);

static int
sem_create (int value)
{
    int semid, ret;

    semid = semget (IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    if ( semid == -1 )
        return -1;

    ret = sem_set_value (semid, value);
    if ( ret < 0 ) {
        sem_delete (semid);
        return -1;
    }

    return semid;
}

static int
sem_delete (int semid)
{
    int ret;

    ret = semctl (semid, 0, IPC_RMID);
    if ( ret < 0 )
        return -1;

    return 0;
}

static int
sem_set_value (int semid, int value)
{
    int ret;

    ret = semctl (semid, 0, SETVAL, value);
    if ( ret < 0 )
        return -1;

    return 0;
}

static int
sem_p (int semid)
{
    struct sembuf semb = {
        .sem_num = 0,
        .sem_op = -1,
        .sem_flg = SEM_UNDO,
    };
    int ret;

    ret = semop (semid, &semb, 1);
    if ( ret < 0 )
        return -1;

    return 0;
}

static int
sem_v (int semid)
{
    struct sembuf semb = {
        .sem_num = 0,
        .sem_op = 1,
        .sem_flg = SEM_UNDO,
    };
    int ret;

    ret = semop (semid, &semb, 1);
    if ( ret < 0 )
        return -1;

    return 0;
}

static int room_semid;
static int chopstick_semid[5];

static void philosoper_proc (int idx)
{
    long rnd_usec;
    srand (time (NULL) + idx);

    while ( 1 ) {
        printf ("Philosopher %d is thinking, NO DISTURB!\n", idx);
        rnd_usec = 1 * 1000000 + rand () % (2 * 1000000);
        usleep (rnd_usec);

        sem_p (room_semid);
        printf ("Philopsopher %d is hungry. He enters the room.\n", idx);

        sem_p (chopstick_semid[idx]);
        printf ("Philopsopher %d picks up his left chopstick.\n", idx);

        sem_p (chopstick_semid[(idx + 1) % 5]);
        printf ("Philopsopher %d picks up his right chopstick.\n", idx);

        printf ("Philopsopher %d starts eating.\n", idx);
        rnd_usec = 3 * 1000000 + rand () % (3 * 1000000);
        usleep (rnd_usec);

        printf ("Philopsopher %d finishes eating.\n", idx);

        sem_v (chopstick_semid[(idx + 1) % 5]);
        printf ("Philopsopher %d puts down his right chopstick.\n", idx);

        sem_v (chopstick_semid[idx]);
        printf ("Philopsopher %d puts down his left chopstick.\n", idx);

        sem_v (room_semid);
        printf ("Philopsopher %d leaves the room.\n", idx);
    }
}

int main ()
{
    pid_t pid;
    pid_t childpid[5] = { 0 };
    int i = 0;
    char ch;

    room_semid = sem_create (4);
    for ( i = 0; i < 5; i++ )
        chopstick_semid[i] = sem_create (1);

    for ( i = 0; i < 5; i++ ) {
        pid = fork ();
        if ( pid < 0 ) {
            printf ("ERROR: Create process %d failed\n.", i);
            goto out;
        }

        if ( pid == 0 ) {
            philosoper_proc (i);
            return 0;
        }

        childpid[i] = pid;
    }

    do {
        ch = getchar ();
    } while ( ch != 'q' );

out:
    for ( i = 0; i < 5; i++ ) {
        if ( childpid[i] != 0 )
            kill (childpid[i], SIGTERM);
    }
    for ( i = 0; i < 5; i++ )
        sem_delete (chopstick_semid[i]);
    sem_delete (room_semid);
    return 0;
}
