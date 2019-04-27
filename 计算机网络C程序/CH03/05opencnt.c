#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define OPENCNT_PATH "opencnt.dat"

static int
read_current_open_count (void)
{
    int fd;
    int opencnt;
    ssize_t ret;

    if ( access (OPENCNT_PATH, F_OK) != 0 ) {
        printf ("File not exist.\n");
        return 0;
    }

    fd = open (OPENCNT_PATH, O_RDONLY);
    if ( fd < 0 ) {
        printf ("File cannot be opened.\n");
        return 0;
    }

    ret = read (fd, &opencnt, sizeof (int));
    if ( ret < sizeof (int)) {
        printf ("Data cannot be read.\n");
        close (fd);
        return 0;
    }

    close (fd);
    return opencnt;
}

static int
write_current_open_count (int opencnt)
{
    int fd;
    ssize_t ret;

    fd = open (OPENCNT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if ( fd < 0 ) {
        printf ("File cannot be opened for writing back.\n");
        return -1;
    }

    ret = write (fd, &opencnt, sizeof (int));
    if ( ret < sizeof (int)) {
        printf ("Data cannot be writen.\n");
        close (fd);
        return -1;
    }

    close (fd);
    return 0;
}

int main ()
{
    int opencnt;

    opencnt = read_current_open_count ();

    opencnt++;
    printf ("OpenCount = %d\n", opencnt);

    write_current_open_count (opencnt);
    return 0;
}
