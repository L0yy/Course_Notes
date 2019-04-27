#include "vsysd.h"

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#define VSD_VSYS_STACK_SIZE  (16 * 1024 * 1024)  // 16MB

struct vsd_vsys_daemon_info {
    int vsys_id;
    int pid;
    int read_pipe;
    int write_pipe;
    char *stack;
};

int vsd_user_dom_sock = -1;
struct vsd_vsys_daemon_info *vsysinfo[VSD_VSYS_MAX_NUM][2] = { NULL };

static int vsd_vsys_daemon_main(void *arg);
static int vsd_init_vsys_daemon(struct vsd_vsys_daemon_info *localinfo);

static int vsd_create_daemon_sock();
static int vsd_accept_client();

static int vsd_do_create_vsys(int sock, struct vsd_message *msg);
static int vsd_do_destroy_vsys(int sock, struct vsd_message *msg);
static int vsd_do_run_on_vsys(int sock, struct vsd_message *msg);

static void vsd_vsys_sig_child (int signo);
static int vsd_vsys_do_run(struct vsd_vsys_daemon_info *localinfo,
                           struct vsd_message *msg, struct vsd_message *resmsg);

int main (int argc, char **argv)
{
    int csock;
    int errcnt = 0;
    struct vsd_message msg;
    int i;

    if ( vsd_create_daemon_sock() < 0 )
        return -1;

    printf("OK\n");

    while (1) {
        csock = vsd_accept_client();
        if ( csock < 0 ) {
            if ( errcnt++ < 100 )
                continue;
            else
                break;
        }

        printf("Accept a client!\n");

        vsd_sock_recv_message (csock, &msg);

        vsd_dump_message(&msg);

        if ( msg.head.command == VSD_MSGCMD_CREATE )
            vsd_do_create_vsys(csock, &msg);
        else if ( msg.head.command == VSD_MSGCMD_DESTROY )
            vsd_do_destroy_vsys(csock, &msg);
        else if ( msg.head.command == VSD_MSGCMD_RUN )
            vsd_do_run_on_vsys(csock, &msg);
        else {
            struct vsd_message resmsg;
            resmsg.head.command = VSD_MSGCMD_RESPONSE;
            resmsg.head.vsysid = msg.head.vsysid;
            resmsg.head.reserved = 0;
            resmsg.u.respn_body.command = msg.head.command;
            resmsg.u.respn_body.reserved = 0;
            resmsg.u.respn_body.respn_type = VSD_RESPN_COMMAND_UNKNOWN;
            vsd_sock_send_message(csock, &resmsg);
        }
            
        vsd_clean_message_heap_mem (&msg);
        close (csock);
    }
    close (vsd_user_dom_sock);
    return 0;
}

static int 
vsd_vsys_daemon_main(void *arg)
{
    char buf[1024];
    
    struct vsd_vsys_daemon_info *localinfo = (struct vsd_vsys_daemon_info *)arg;
    struct vsd_message msg, resmsg;
    int ret, runflag = 1;
    int runpid;
    char pidpath[32];
    
    signal (SIGCHLD, vsd_vsys_sig_child);
    
    printf("A new vsys is created:\n");
    printf("  Vsys: %d\n", localinfo->vsys_id);
    printf("  PID:  %d\n", localinfo->pid);
    printf("  Pipe: <R:%d> <W:%d>\n", localinfo->read_pipe, localinfo->write_pipe);
    
    vsd_init_vsys_daemon (localinfo);
    mount("proc", "/proc", "proc", 0, NULL);
   
    system("ps aux");
    vsd_make_message_magic_field (&resmsg.head);
    resmsg.head.command = VSD_MSGCMD_RESPONSE;
    resmsg.head.vsysid = localinfo->vsys_id;
    resmsg.head.reserved = 0;
    resmsg.u.respn_body.command = VSD_MSGCMD_CREATE;
    resmsg.u.respn_body.reserved = 0;
    resmsg.u.respn_body.respn_type = VSD_RESPN_VSYS_CREATE_SUCCESS;
    vsd_pipe_send_message(localinfo->write_pipe, &resmsg);
    
    while ( runflag ) {
        ret = vsd_pipe_recv_message(localinfo->read_pipe, &msg);
        if ( ret < 0 ) {
            printf("Vsys-%d, a corrupted message received\n", (int)localinfo->vsys_id);
            continue;
        }
        
        ret = vsd_check_message_magic_field (&msg.head);
        if ( ret < 0 ) {
            printf("Vsys-%d, message format error\n", (int)localinfo->vsys_id);
            continue;
        }
        
        if ( msg.head.vsysid != localinfo->vsys_id ) {
            printf("Vsys-%d, message target error (to vsys-%d)\n", (int)localinfo->vsys_id, (int)msg.head.vsysid);
            continue;
        }
        
        printf("Vsys-%d, %s command received!\n", 
               (int)localinfo->vsys_id, vsd_get_cmdtype_str(msg.head.command));
        switch ( msg.head.command ) {
          case VSD_MSGCMD_CREATE:
            printf("    Redundant create command!\n");
            break;
            
          case VSD_MSGCMD_DESTROY:
            runflag = 0;
            resmsg.u.respn_body.command = VSD_MSGCMD_DESTROY;
            resmsg.u.respn_body.respn_type = VSD_RESPN_VSYS_DESTROY_SUCCESS;
            vsd_pipe_send_message(localinfo->write_pipe, &resmsg);
            break;
          
          case VSD_MSGCMD_RUN:
            vsd_vsys_do_run(localinfo, &msg, &resmsg);
            vsd_pipe_send_message(localinfo->write_pipe, &resmsg);
            break;
          
          defalut:
            printf("    Non-implemented command!\n");
            break;
        }
        
        vsd_clean_message_heap_mem (&msg);
    }
    
    close (localinfo->read_pipe);
    close (localinfo->write_pipe);
    //exit (0);
    return 0;
}

static int 
vsd_init_vsys_daemon(struct vsd_vsys_daemon_info *localinfo)
{
    int i;
    
    close (vsd_user_dom_sock);
    
    for ( i = 0; i < VSD_VSYS_MAX_NUM; i++ ) {
        if ( vsysinfo[i][0] == NULL )
            continue;
        
        // NOTE: This will free the resource for current vsys, but they are used for 
        //       parent process, the child can free them safely.
        close (vsysinfo[i][0]->read_pipe);
        close (vsysinfo[i][0]->write_pipe);
    }
    return 0;
}

static int 
vsd_create_daemon_sock()
{
    struct sockaddr_un addr;
    int len;

    if ( vsd_user_dom_sock >= 0 ) {
        printf ("vsys error: create a redundant connection!\n");
        return -1;
    }

    vsd_user_dom_sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if ( vsd_user_dom_sock < 0 ) {
        printf ("vsys error: cannot create domain socket!\n");
        return -1;
    }

    unlink (VSD_USR_DOMSK_PATH);

    memset (&addr, 0, sizeof (struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy (addr.sun_path, VSD_USR_DOMSK_PATH, strlen (VSD_USR_DOMSK_PATH));
    len = sizeof (addr.sun_family) + strlen (addr.sun_path);

    if ( bind (vsd_user_dom_sock, (struct sockaddr *) &addr, len) < 0 ) {
        printf ("vsys error: cannot bind on address!\n");
        close (vsd_user_dom_sock);
        return -1;
    }

    if ( listen (vsd_user_dom_sock, 5) < 0 ) {
        printf ("vsys error: cannot listen on the address!\n");
        close (vsd_user_dom_sock);
        return -1;
    }

    return vsd_user_dom_sock;
}

static int 
vsd_accept_client()
{
    int csock;
    struct sockaddr_un addr;
    int len;

    if ( vsd_user_dom_sock < 0 ) {
        printf ("vsys error: Socket not ready!\n");
        return -1;
    }

    csock = accept (vsd_user_dom_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
    if ( csock < 0 ) {
        printf ("Error when accepting...%s\n", strerror (errno));
        return -1;
    }

    return csock;
}

static inline void
__vsd_clean_vsys_resource (int vsysid)
{
    close (vsysinfo[vsysid][0]->read_pipe);
    close (vsysinfo[vsysid][0]->write_pipe);
    free (vsysinfo[vsysid][0]->stack);
    free (vsysinfo[vsysid][0]);
    vsysinfo[vsysid][0] = NULL;
    
    free (vsysinfo[vsysid][1]);
    vsysinfo[vsysid][1] = NULL;
}

static int 
vsd_do_create_vsys(int sock, struct vsd_message *msg)
{
    int vsysid = msg->head.vsysid, chdpid;
    int comm_pipe[2][2] = { { -1, -1 }, { -1, -1 } };
    int ret[2];
    struct vsd_vsys_daemon_info *vsinfo = NULL;
    char *chdstack = NULL;
    struct vsd_message resmsg;
    
    if ( vsysinfo[vsysid][0] != NULL ) {
        printf ("Parent: Vsys-%d has already existed!\n");
        return -1;
    }
    
    ret[0] = pipe (comm_pipe[0]);
    ret[1] = pipe (comm_pipe[1]);
    if ( ret[0] < 0 || ret[1] < 0 ) {
        if ( comm_pipe[0][0] != -1 )
            close (comm_pipe[0][0]);
        if ( comm_pipe[0][1] != -1 )
            close (comm_pipe[0][1]);
        if ( comm_pipe[1][0] != -1 )
            close (comm_pipe[1][0]);
        if ( comm_pipe[1][1] != -1 )
            close (comm_pipe[1][1]);
            
        printf("Parent: Cannot create pipeline between vsys.\n");
        return -1;
    }
        
    vsinfo = malloc (sizeof (struct vsd_vsys_daemon_info));
    if ( vsinfo == NULL ) {
        printf ("Parent: Cannot create info memory!\n");
        goto VSD_CREATE_VSYS_ERR_PROC;
    }
    
    vsinfo->vsys_id = vsysid;
    vsinfo->pid = 1;
    vsinfo->read_pipe = comm_pipe[0][0];
    vsinfo->write_pipe = comm_pipe[1][1];
    vsinfo->stack = NULL;
   
    chdstack = malloc (VSD_VSYS_STACK_SIZE);
    if ( vsinfo == NULL ) {
        printf ("Parent: Cannot create stack for child!\n");
        goto VSD_CREATE_VSYS_ERR_PROC;
    }
    
    vsysinfo[vsysid][0] = malloc (sizeof (struct vsd_vsys_daemon_info));
    if ( vsysinfo[vsysid][0] == NULL ) {
        printf ("Parent: Cannot create database info memory!\n");
        goto VSD_CREATE_VSYS_ERR_PROC;
    }
    vsysinfo[vsysid][0]->vsys_id = vsysid;
    vsysinfo[vsysid][0]->read_pipe = comm_pipe[1][0];
    vsysinfo[vsysid][0]->write_pipe = comm_pipe[0][1];  
    vsysinfo[vsysid][0]->stack = chdstack; 
    
    vsysinfo[vsysid][1] = vsinfo;            

    chdpid = clone (vsd_vsys_daemon_main,
                    chdstack + VSD_VSYS_STACK_SIZE,
                    CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD,
                    vsinfo);
    if ( chdpid < 0 ) {
        printf ("Parent: Cannot create child process!\n");
        goto VSD_CREATE_VSYS_ERR_PROC;
    }       
    
    close (vsinfo->read_pipe);
    close (vsinfo->write_pipe);
    vsysinfo[vsysid][0]->pid = chdpid;
    
    ret[0] = vsd_pipe_recv_message(vsysinfo[vsysid][0]->read_pipe, &resmsg);
    if ( ret[0] >= 0 ) {
        ret[1] = vsd_sock_send_message(sock, &resmsg);
        ret[0] = vsd_check_response_msg (VSD_MSGCMD_CREATE, &resmsg);
    }
    if ( ret[0] < 0 || ret[1] < 0 ) {
        printf("Parent: Created Vsys-%d (child pid = %d) FAILED!!\n", 
               vsysid, vsysinfo[vsysid][0]->pid);
        
        __vsd_clean_vsys_resource (vsysid);
        printf ("Parent: Vsys %d is destroyed!\n", vsysid);
        
        return ret[0];
    }

    printf("Parent: Created Vsys-%d (child pid = %d)\n", vsysid, vsysinfo[vsysid][0]->pid);                      
    return 0;
    
VSD_CREATE_VSYS_ERR_PROC:
    close (comm_pipe[0][0]);
    close (comm_pipe[0][1]);
    close (comm_pipe[1][0]);
    close (comm_pipe[1][1]);
    
    if ( vsinfo != NULL )
        free (vsinfo);
    if ( vsysinfo[vsysid][0] != NULL ) {
        free (vsysinfo[vsysid][0]);
        vsysinfo[vsysid][0] = NULL;
    }
    if ( chdstack != NULL )
        free (chdstack);
        
    return -1;
}

static int 
vsd_do_destroy_vsys(int sock, struct vsd_message *msg)
{
    int vsysid = msg->head.vsysid;
    int ret;
    struct vsd_message resmsg;
    
    if ( vsysinfo[vsysid][0] == NULL ) {
        printf("Parent: No Vsys %d detected!\n", vsysid);
        return -1;
    }
    
    ret = vsd_pipe_send_message(vsysinfo[vsysid][0]->write_pipe, msg);
    if ( ret < 0 ) {
        printf("Parent: cannot send command to vsys!\n");
        return -1;
    }
    
    ret = vsd_pipe_recv_message(vsysinfo[vsysid][0]->read_pipe, &resmsg);
    if ( ret < 0 ) {
        printf("Parent: cannot recv respnose msg from vsys!\n");
        return -1;
    }
    
    ret = vsd_check_response_msg (VSD_MSGCMD_DESTROY, &resmsg);
    if ( ret < 0 ) {
        vsd_sock_send_message(sock, &resmsg);
        printf("Parent: respnose msg reports error!\n");
        return -1;
    }
    
    waitpid (vsysinfo[vsysid][0]->pid, NULL, 0);
    
    __vsd_clean_vsys_resource (vsysid);
    printf ("Parent: Vsys %d is destroyed!\n", vsysid);
    
    vsd_sock_send_message(sock, &resmsg);
    return 0;
}

static int 
vsd_do_run_on_vsys(int sock, struct vsd_message *msg)
{
    int vsysid = msg->head.vsysid;
    int ret;
    struct vsd_message resmsg;
    
    if ( vsysinfo[vsysid][0] == NULL ) {
        printf("Parent: No Vsys %d detected!\n", vsysid);
        return -1;
    }
    
    ret = vsd_pipe_send_message(vsysinfo[vsysid][0]->write_pipe, msg);
    if ( ret < 0 ) {
        printf("Parent: cannot send command to vsys!\n");
        return -1;
    }
    
    ret = vsd_pipe_recv_message(vsysinfo[vsysid][0]->read_pipe, &resmsg);
    if ( ret < 0 ) {
        printf("Parent: cannot recv respnose msg from vsys!\n");
        return -1;
    }
    vsd_sock_send_message(sock, &resmsg);

    ret = vsd_check_response_msg (VSD_MSGCMD_RUN, &resmsg);
    if ( ret < 0 ) {
        printf("Parent: respnose msg reports error!\n");
        return -1;
    }
    
    return 0;
}

static void vsd_vsys_sig_child (int signo)
{
    pid_t pid;
    int stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0 )
        printf("child %d terminated.\n", pid);
}

static int vsd_vsys_do_run(struct vsd_vsys_daemon_info *localinfo,
                           struct vsd_message *msg, struct vsd_message *resmsg)
{
    int runpid, ret, i;
    char pidpath[32], pidfile[32];
    int pidpathfd;
    
    resmsg->u.respn_body.command = VSD_MSGCMD_RUN;
    runpid = fork();
    if ( runpid < 0 ) {
        resmsg->u.respn_body.respn_type = VSD_RESPN_VSYS_EXEC_START_FAILED;
        goto VSD_VSRUN_EXIT;
    }
    
    if ( runpid == 0 ) { // Child Process
        ret = setsid();
        
        if ( ret >= 0 )
            ret = execvp (msg->u.exec_body.argv[0], msg->u.exec_body.argv);
            
        if ( ret < 0 ) {
            // Sleep for a while to make the parent to experience the error.
            sleep (1);
            exit(-1);
        } else {
            exit(0);
        }

    } else { // Parent Process
        sprintf(pidpath, "/proc/%d/stat", runpid);
        
        for ( i = 0; i < 3; i++ ) {
            usleep (1000);
            
            pidpathfd = open (pidpath, O_RDWR, 0);
            if ( pidpathfd < 0 ) {
                resmsg->u.respn_body.respn_type = VSD_RESPN_VSYS_EXEC_START_SUCCESS;
                goto VSD_VSRUN_EXIT;
            }
            
            ret = read (pidpathfd, pidfile, 31);
            pidfile[31] = '\0';
            if ( ret < 0 ) {
                resmsg->u.respn_body.respn_type = VSD_RESPN_VSYS_EXEC_START_SUCCESS;
                goto VSD_VSRUN_EXIT;
            }
        
            if ( strstr (pidfile, "(vsysd)") == NULL ) {
                resmsg->u.respn_body.respn_type = VSD_RESPN_VSYS_EXEC_START_SUCCESS;
                goto VSD_VSRUN_EXIT;
            } else {
                resmsg->u.respn_body.respn_type = VSD_RESPN_VSYS_EXEC_START_FAILED;
            }
            close (pidpathfd);
        }
    }

VSD_VSRUN_EXIT:
    return 0;
}