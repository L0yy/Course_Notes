#include "vsysd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

static int vsc_sock = -1;

static int vsc_do_command (int argc, char **argv);
static int vsc_do_cmd_create (int sock, int argc, char **argv);
static int vsc_do_cmd_destroy (int sock, int argc, char **argv);
static int vsc_do_cmd_run (int sock, int argc, char **argv);
static int vsc_do_cmd_linkmove (int sock, int argc, char **argv);

static int vsc_connect_to_daemon ();

int main (int argc, char **argv)
{
    if ( vsc_do_command(argc - 1, argv + 1) < 0 )
        return -1;

    printf("OK\n");
    //close (vsc_sock);
    return 0;
}

static int vsc_do_command(int argc, char **argv)
{
    int cmdtype, ret;
    int csock;

    if ( argc < 1 ) {
        printf ("vsys error: No command specified!\n");
        return -1;
    }

    cmdtype = vsd_parse_cmdtype_str(argv[0]);
    if ( cmdtype < 0 ) {
        printf("vsys error: unknown command!\n");
        return -1;
    }

    csock = vsc_connect_to_daemon();
    if ( csock < 0 )
        return -1;

    switch (cmdtype) {
      case VSD_MSGCMD_CREATE:
        printf("vsys info: do create command.\n");
        ret = vsc_do_cmd_create (csock, argc - 1, argv + 1);
        break;

      case VSD_MSGCMD_DESTROY:
        printf("vsys info: do destroy command.\n");
        ret = vsc_do_cmd_destroy (csock, argc - 1, argv + 1);
        break;

      case VSD_MSGCMD_RUN:
        printf("vsys info: do run command.\n");
        ret = vsc_do_cmd_run (csock, argc - 1, argv + 1);
        break;

      case VSD_MSGCMD_LNK_MOVE:
        printf("vsys info: do link move command.\n");
        ret = vsc_do_cmd_linkmove (csock, argc - 1, argv + 1);
        break;

      default:
        printf("vsys error: command cannot be handled!\n");
        ret = -1;
        break;
    }

    close (csock);
    return ret;
}

static int vsc_do_cmd_create (int sock, int argc, char **argv)
{
    int vsysid, ret;
    struct vsd_message msg, resmsg;

    if ( argc < 1 ) {
        printf ("vsys error: No vsys is specified!\n");
        return -1;
    }

    vsysid = vsd_parse_vsys_str (argv[0]);
    if ( vsysid == -1 ) {
        printf ("vsys error: Invalid vsys id!\n");
        return -1;
    }

    memset (&msg, 0, sizeof (struct vsd_message));
    vsd_make_message_magic_field (&msg.head);
    msg.head.command = VSD_MSGCMD_CREATE;
    msg.head.vsysid = (unsigned char)vsysid;

    ret = vsd_sock_send_message (sock, &msg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot send msg to server!\n");
        return -1;
    }
    
    ret = vsd_sock_recv_message (sock, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot receive response from server!\n");
        return -1;
    }
    
    printf ("vsys info: Response msg received.\n");
    vsd_dump_message (&resmsg);
    ret = vsd_check_response_msg (VSD_MSGCMD_CREATE, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: response message reports error (errcode=%d)!\n", 
                (int)resmsg.u.respn_body.respn_type);
        return -1;
    }
    
    return 0;
}

static int vsc_do_cmd_destroy (int sock, int argc, char **argv)
{
    int vsysid, ret;
    struct vsd_message msg, resmsg;

    if ( argc < 1 ) {
        printf ("vsys error: No vsys is specified!\n");
        return -1;
    }

    vsysid = vsd_parse_vsys_str (argv[0]);
    if ( vsysid == -1 ) {
        printf ("vsys error: Invalid vsys id!\n");
        return -1;
    }

    memset (&msg, 0, sizeof (struct vsd_message));
    vsd_make_message_magic_field (&msg.head);
    msg.head.command = VSD_MSGCMD_DESTROY;
    msg.head.vsysid = (unsigned char)vsysid;

    ret = vsd_sock_send_message (sock, &msg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot send msg to server!\n");
        return -1;
    }
    
    ret = vsd_sock_recv_message (sock, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot receive response from server!\n");
        return -1;
    }
    
    printf ("vsys info: Response msg received.\n");
    vsd_dump_message (&resmsg);
    ret = vsd_check_response_msg (VSD_MSGCMD_DESTROY, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: response message reports error (errcode=%d)!\n", 
                (int)resmsg.u.respn_body.respn_type);
        return -1;
    }
    
    return 0;
}

static int vsc_do_cmd_run (int sock, int argc, char **argv)
{
    int vsysid, ret;
    struct vsd_message msg, resmsg;

    if ( argc < 2 ) {
        printf ("vsys error: No vsys or execution command is specified!\n");
        return -1;
    }

    vsysid = vsd_parse_vsys_str (argv[0]);
    if ( vsysid == -1 ) {
        printf ("vsys error: Invalid vsys id!\n");
        return -1;
    }

    memset (&msg, 0, sizeof (struct vsd_message));
    vsd_make_message_magic_field (&msg.head);
    msg.head.command = VSD_MSGCMD_RUN;
    msg.head.vsysid = (unsigned char)vsysid;
    msg.u.exec_body.argc = argc - 1;
    msg.u.exec_body.argv = argv + 1;

    ret = vsd_sock_send_message (sock, &msg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot send msg to server!\n");
        return -1;
    }
    
    ret = vsd_sock_recv_message (sock, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot receive response from server!\n");
        return -1;
    }
    
    printf ("vsys info: Response msg received.\n");
    vsd_dump_message (&resmsg);
    ret = vsd_check_response_msg (VSD_MSGCMD_RUN, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: response message reports error (errcode=%d)!\n", 
                (int)resmsg.u.respn_body.respn_type);
        return -1;
    }
    
    return 0;
}

static int vsc_do_cmd_linkmove (int sock, int argc, char **argv)
{
    int vsysid, dst_vsysid, ret;
    struct vsd_message msg, resmsg;

    if ( argc < 3 ) {
        printf ("vsys error: No vsys or link name is specified!\n");
        return -1;
    }

    vsysid = vsd_parse_vsys_str (argv[0]);
    if ( vsysid == -1 ) {
        printf ("vsys error: Invalid vsys id!\n");
        return -1;
    }

    dst_vsysid = vsd_parse_vsys_str (argv[2]);
    if ( dst_vsysid == -1 ) {
        printf ("vsys error: Invalid dst vsys id!\n");
        return -1;
    }

    memset (&msg, 0, sizeof (struct vsd_message));
    vsd_make_message_magic_field (&msg.head);
    msg.head.command = VSD_MSGCMD_LNK_MOVE;
    msg.head.vsysid = (unsigned char)vsysid;
    msg.u.lnkmv_body.link_name = argv[1];
    msg.u.lnkmv_body.dst_vsys = dst_vsysid;

    ret = vsd_sock_send_message (sock, &msg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot send msg to server!\n");
        return -1;
    }
    
    ret = vsd_sock_recv_message (sock, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: cannot receive response from server!\n");
        return -1;
    }
    
    printf ("vsys info: Response msg received.\n");
    vsd_dump_message (&resmsg);
    ret = vsd_check_response_msg (VSD_MSGCMD_LNK_MOVE, &resmsg);
    if ( ret < 0 ) {
        printf ("vsys error: response message reports error (errcode=%d)!\n", 
                (int)resmsg.u.respn_body.respn_type);
        return -1;
    }
    
    return 0;
}

static int vsc_connect_to_daemon()
{
    struct sockaddr_un addr;
    int len;

    if ( vsc_sock >= 0 ) {
        printf ("vsys error: create a redundant connection!\n");
        return -1;
    }

    vsc_sock = socket (AF_UNIX, SOCK_STREAM, 0);
    if ( vsc_sock < 0 ) {
        printf ("vsys error: cannot create domain socket!\n");
        return -1;
    }

    memset (&addr, 0, sizeof (struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy (addr.sun_path, VSD_USR_DOMSK_PATH, strlen (VSD_USR_DOMSK_PATH));
    len = sizeof (addr.sun_family) + strlen (addr.sun_path);

    if ( connect (vsc_sock, (struct sockaddr *) &addr, len) < 0 ) {
        printf ("vsys error: cannot connect to daemon!\n");
        close (vsc_sock);
    }

    return vsc_sock;
}
