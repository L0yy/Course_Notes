#include "vsysd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

const struct __vsd_message_cmdstr_map {
    int cmdtype;
    char *cmdstr;
} __vsd_msg_cmdstr_map[] = {
    { VSD_MSGCMD_CREATE,   "create"    },
    { VSD_MSGCMD_DESTROY,  "destroy"   },
    { VSD_MSGCMD_RUN,      "run"       },
    { VSD_MSGCMD_LNK_MOVE, "link-move" }
};

#define __VSD_MESSAGE_CMD_NUM    \
        (sizeof (__vsd_msg_cmdstr_map) / sizeof (__vsd_msg_cmdstr_map[0]))

int
vsd_get_cmdtype_num ()
{
    return __VSD_MESSAGE_CMD_NUM;
}

int 
vsd_parse_cmdtype_str (char *cmdstr)
{
    int i;

    if ( cmdstr == NULL || strlen (cmdstr) == 0 )
        return -1;

    for ( i = 0; i < __VSD_MESSAGE_CMD_NUM; i++ ) {
        if ( strcmp (cmdstr, __vsd_msg_cmdstr_map[i].cmdstr) == 0 )
            return __vsd_msg_cmdstr_map[i].cmdtype;
    }
    return -1;
}

const char *
vsd_get_cmdtype_str (int cmdtype)
{
    int i;
    for ( i = 0; i < __VSD_MESSAGE_CMD_NUM; i++ ) {
        if ( __vsd_msg_cmdstr_map[i].cmdtype == cmdtype )
            return __vsd_msg_cmdstr_map[i].cmdstr;
    }
    return "";
}

int
vsd_parse_vsys_str (char *vsysstr)
{
    char *p = vsysstr;
    int vsysid = 0;

    while ( *p != '\0' ) {
        if ( *p < '0' || *p > '9' )
            return -1;
        vsysid = vsysid * 10 + *p - '0';
        p++;
    }
    if ( vsysid >= VSD_VSYS_MAX_NUM )
        return -1;
    else
        return vsysid;
}

enum __vsd_message_info_choice {
    __VSD_SMI_CH_NONE,
    __VSD_SMI_CH_LINK_MOVE,
    __VSD_SMI_CH_ARGCnV,
    __VSD_SMI_CH_RESPN
};

static inline int 
vsd_get_message_info_choice (int cmdtype)
{
    if ( cmdtype == VSD_MSGCMD_RUN )
        return __VSD_SMI_CH_ARGCnV;
    else if ( cmdtype == VSD_MSGCMD_LNK_MOVE )
        return __VSD_SMI_CH_LINK_MOVE;
    else if ( cmdtype == VSD_MSGCMD_RESPONSE )
        return __VSD_SMI_CH_RESPN;
    else
        return __VSD_SMI_CH_NONE;
}

static const unsigned char __vsd_magic_field[4] = {
    0xFF, 0xAA, 0xAA, 0xFF
};

int 
vsd_make_message_magic_field (struct vsd_message_head *msghead)
{
    memcpy (msghead->magic_field, __vsd_magic_field, 4);
    return 0;
}

int
vsd_check_message_magic_field (struct vsd_message_head *msghead)
{
    if ( memcmp (msghead->magic_field, __vsd_magic_field, 4) != 0 )
        return -1;
    return 0;
}

int 
vsd_check_response_msg (int command, struct vsd_message *msg)
{
    if ( msg == NULL || msg->head.command != VSD_MSGCMD_RESPONSE )
        return -1;
    
    if ( command != msg->u.respn_body.command )
        return -1;
    
    if ( msg->u.respn_body.command == VSD_MSGCMD_CREATE &&
         msg->u.respn_body.respn_type == VSD_RESPN_VSYS_CREATE_SUCCESS )
        return 0;
    else if ( msg->u.respn_body.command == VSD_MSGCMD_DESTROY &&
              msg->u.respn_body.respn_type == VSD_RESPN_VSYS_DESTROY_SUCCESS )
        return 0;
    else if ( msg->u.respn_body.command == VSD_MSGCMD_RUN &&
              (msg->u.respn_body.respn_type == VSD_RESPN_VSYS_EXEC_START_SUCCESS ||
               msg->u.respn_body.respn_type == VSD_RESPN_VSYS_EXEC_FINISHED) )
        return 0;
    else if ( msg->u.respn_body.command == VSD_MSGCMD_LNK_MOVE &&
              msg->u.respn_body.respn_type == VSD_RESPN_VSYS_LINK_MOVED )
        return 0;

    return -1;  
}

void
vsd_clean_message_heap_mem (struct vsd_message *msg)
{
    int infoch, i;
    
    infoch = vsd_get_message_info_choice (msg->head.command);
    switch ( infoch ) {
      case __VSD_SMI_CH_LINK_MOVE:
        free (msg->u.lnkmv_body.link_name);
        break;
        
      case __VSD_SMI_CH_ARGCnV:
        for ( i = 0; i < msg->u.exec_body.argc; i++ ) {
            free (msg->u.exec_body.argv[i]);
        }
        free (msg->u.exec_body.argv);
        break;
        
      default:
        break;
    }  
}

void 
vsd_dump_message (struct vsd_message *msg)
{
    int infoch, i;
    
    printf("Head: [%02X %02X %02X %02X] [%02X] [%02X] [%04X]\n", 
           msg->head.magic_field[0], msg->head.magic_field[1], msg->head.magic_field[2], msg->head.magic_field[3],
           msg->head.command, msg->head.vsysid, msg->head.reserved);

    infoch = vsd_get_message_info_choice (msg->head.command);
    switch ( infoch ) {
      case __VSD_SMI_CH_LINK_MOVE:
        printf ("  LnkMvBody: %s --> Vsys-%d\n", msg->u.lnkmv_body.link_name, (int)msg->u.lnkmv_body.dst_vsys);
        break;
        
      case __VSD_SMI_CH_ARGCnV:
        printf ("  ExecBody: [%d] ", msg->u.exec_body.argc);
        for ( i = 0; i < msg->u.exec_body.argc; i++ ) {
            printf ("%s ", msg->u.exec_body.argv[i]);
        }
        printf("\n");
        break;
        
      case __VSD_SMI_CH_RESPN:
        printf("  RespnBody: Cmd[%02X] [%02X] Res[%04X]\n", 
               msg->u.respn_body.command, msg->u.respn_body.reserved, msg->u.respn_body.respn_type);
        break;
        
      default:
        break;
    }  
}

static int
vsd_sock_readn (int sock, void *bufptr, int len)
{
    int nleft = len;
    int nread;
    unsigned char *ptr = bufptr;

    while ( nleft > 0 ) {
        nread = read (sock, ptr, nleft);

        if ( nread < 0 ) {
            switch (errno) {
              case EINTR:
              case EAGAIN:
              case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
              case EWOULDBLOCK:
#endif /* (EWOULDBLOCK != EAGAIN) */
                sleep (0);
                continue;
            }
            return (nread);
        } else {
            if ( nread == 0 ) 
                break;
        }

        nleft -= nread;
        ptr += nread;
    }
    return len - nleft;
}  


static int
vsd_sock_writen (int sock, void *bufptr, int len)
{
    int nleft = len;
    int nwritten;
    unsigned char *ptr = bufptr;

    while (nleft > 0) {
        nwritten = write (sock, ptr, nleft);
      
        if (nwritten <= 0) {
            /* Signal happened before we could write */
            switch (errno) {
              case EINTR:
              case EAGAIN:
              case EINPROGRESS:
#if (EWOULDBLOCK != EAGAIN)
              case EWOULDBLOCK:
#endif /* (EWOULDBLOCK != EAGAIN) */
                sleep (0);
                continue;
            }
            return (nwritten);
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return len - nleft;
}

static int 
vsd_sock_send_str (int sock, char *str)
{
    int datlen = strlen (str);
    int sendlen;
    
    sendlen = vsd_sock_writen (sock, &datlen, sizeof (int));
    if ( sendlen < sizeof (int) ) {
        return -1;
    }
    
    sendlen = vsd_sock_writen (sock, str, datlen);
    if ( sendlen < datlen )
        return -1;
    
    return sendlen;
}

static int 
vsd_sock_recv_str (int sock, char **pstr)
{
    int datlen = 0;
    int recvlen;
    
    recvlen = vsd_sock_readn (sock, &datlen, sizeof (int));
    if ( recvlen < sizeof (int) ) {
        return -1;
    }
    
    *pstr = malloc (datlen + 1);
    if ( *pstr == NULL )
        return -1;
        
    (*pstr)[datlen] = '\0';
    recvlen = vsd_sock_readn (sock, *pstr, datlen);
    if ( recvlen < datlen )
        return -1;
    
    return recvlen;
}

int 
vsd_sock_send_message(int sock, struct vsd_message *msg)
{
    int ret, infoch, i;
    
    ret = vsd_sock_writen (sock, &msg->head, sizeof (struct vsd_message_head));
    if ( ret < 0 ) {
        printf ("vsys error: cannot write head data to socket!\n");
        return -1;
    }

    infoch = vsd_get_message_info_choice (msg->head.command);
    switch ( infoch ) {
      case __VSD_SMI_CH_LINK_MOVE:
        ret = vsd_sock_writen (sock, &msg->u.lnkmv_body.dst_vsys, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (LNKMV:dst_vsys) data to socket!\n");
            return -1;
        }
        
        ret = vsd_sock_send_str (sock, msg->u.lnkmv_body.link_name);
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (LNKMV:lnk_name) data to socket!\n");
            return -1;
        }
        break;
        
      case __VSD_SMI_CH_ARGCnV:
        ret = vsd_sock_writen (sock, &msg->u.exec_body.argc, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (EXEC:argc) data to socket!\n");
            return -1;
        }
        
        for ( i = 0; i < msg->u.exec_body.argc; i++ ) {
            ret = vsd_sock_send_str (sock, msg->u.exec_body.argv[i]);
            if ( ret < 0 ) {
                printf ("vsys error: cannot write body (EXEC:argv[%d]) data to socket!\n", i);
                return -1;
            }
        }
        break;
      
      case __VSD_SMI_CH_RESPN:
        ret = vsd_sock_writen (sock, &msg->u.respn_body, sizeof (struct vsd_respn_msg_body));
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (RESPN) data to socket!\n");
            return -1;
        }
        break;
          
      default:
        break;
    }

    return 0;
}

int 
vsd_sock_recv_message(int sock, struct vsd_message *msg)
{
    int ret, infoch, i;
    if ( msg == NULL )
        return -1;

    ret = vsd_sock_readn (sock, &msg->head, sizeof (struct vsd_message_head));
    if ( ret < 0 ) {
        printf ("vsys error: cannot read head data from socket!\n");
        return -1;
    }

    infoch = vsd_get_message_info_choice (msg->head.command);
    switch ( infoch ) {
      case __VSD_SMI_CH_LINK_MOVE:
        ret = vsd_sock_readn (sock, &msg->u.lnkmv_body.dst_vsys, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (LNKMV:dst_vsys) data from socket!\n");
            return -1;
        }
        
        ret = vsd_sock_recv_str (sock, &msg->u.lnkmv_body.link_name);
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (LNKMV:link_name) data from socket!\n");
            return -1;
        }
        
        break;

      case __VSD_SMI_CH_ARGCnV:
        ret = vsd_sock_readn (sock, &msg->u.exec_body.argc, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (EXEC:argc) data from socket!\n");
            return -1;
        }
        
        msg->u.exec_body.argv = malloc ((msg->u.exec_body.argc + 1) * sizeof (char *));
        if ( msg->u.exec_body.argv == NULL ) {
            printf ("vsys error: cannot alloc body (EXEC:argv) data!\n");
            return -1;
        }
        msg->u.exec_body.argv[msg->u.exec_body.argc] = NULL;

        for ( i = 0; i < msg->u.exec_body.argc; i++ ) {
            ret = vsd_sock_recv_str (sock, &msg->u.exec_body.argv[i]);
            if ( ret < 0 ) {
                printf ("vsys error: cannot read body (EXEC:argv[%d]) data from socket!\n", i);
                return -1;
            }
        }
        break;

      case __VSD_SMI_CH_RESPN:
        ret = vsd_sock_readn (sock, &msg->u.respn_body, sizeof (struct vsd_respn_msg_body));
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (RESPN) data from socket!\n");
            return -1;
        }
        break;

      default:
        break;
    }
    return 0;
}

static inline int
vsd_pipe_writen (int pipefd, void *bufptr, int len)
{
    return write (pipefd, bufptr, len);
}

static inline int
vsd_pipe_readn (int pipefd, void *bufptr, int len)
{
    int readlen = 0;
    read (pipefd, bufptr, len);
}

static int 
vsd_pipe_send_str (int pipefd, char *str)
{
    int datlen = strlen (str);
    int sendlen;
    
    sendlen = vsd_pipe_writen (pipefd, &datlen, sizeof (int));
    if ( sendlen < sizeof (int) ) {
        return -1;
    }
    
    sendlen = vsd_pipe_writen (pipefd, str, datlen);
    if ( sendlen < datlen )
        return -1;
    
    return sendlen;
}

static int 
vsd_pipe_recv_str (int pipefd, char **pstr)
{
    int datlen = 0;
    int recvlen;
    
    recvlen = vsd_pipe_readn (pipefd, &datlen, sizeof (int));
    if ( recvlen < sizeof (int) ) {
        return -1;
    }
    
    *pstr = malloc (datlen + 1);
    if ( *pstr == NULL )
        return -1;
    
    (*pstr)[datlen] = '\0';
    recvlen = vsd_pipe_readn (pipefd, *pstr, datlen);
    if ( recvlen < datlen )
        return -1;
    
    return recvlen;
}

int 
vsd_pipe_send_message(int pipefd, struct vsd_message *msg)
{
    int ret, infoch, i;
    
    ret = vsd_pipe_writen (pipefd, &msg->head, sizeof (struct vsd_message_head));
    if ( ret < 0 ) {
        printf ("vsys error: cannot write head data to pipe!\n");
        return -1;
    }

    infoch = vsd_get_message_info_choice (msg->head.command);
    switch ( infoch ) {
      case __VSD_SMI_CH_LINK_MOVE:
        ret = vsd_pipe_writen (pipefd, &msg->u.lnkmv_body.dst_vsys, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (LNKMV:dst_vsys) data to pipe!\n");
            return -1;
        }
        
        ret = vsd_pipe_send_str (pipefd, msg->u.lnkmv_body.link_name);
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (LNKMV:lnk_name) data to pipe!\n");
            return -1;
        }
        break;
        
      case __VSD_SMI_CH_ARGCnV:
        ret = vsd_pipe_writen (pipefd, &msg->u.exec_body.argc, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (EXEC:argc) data to pipe!\n");
            return -1;
        }
        
        for ( i = 0; i < msg->u.exec_body.argc; i++ ) {
            ret = vsd_pipe_send_str (pipefd, msg->u.exec_body.argv[i]);
            if ( ret < 0 ) {
                printf ("vsys error: cannot write body (EXEC:argv[%d]) data to pipe!\n", i);
                return -1;
            }
        }
        break;
        
      case __VSD_SMI_CH_RESPN:
        ret = vsd_pipe_writen (pipefd, &msg->u.respn_body, sizeof (struct vsd_respn_msg_body));
        if ( ret < 0 ) {
            printf ("vsys error: cannot write body (RESPN) data to pipe!\n");
            return -1;
        }
        break;

      default:
        break;
    }

    return 0;
}

int 
vsd_pipe_recv_message(int pipefd, struct vsd_message *msg)
{
    int ret, infoch, i;
    if ( msg == NULL )
        return -1;

    ret = vsd_pipe_readn (pipefd, &msg->head, sizeof (struct vsd_message_head));
    if ( ret < 0 ) {
        printf ("vsys error: cannot read head data from pipe!\n");
        return -1;
    }

    infoch = vsd_get_message_info_choice (msg->head.command);
    switch ( infoch ) {
      case __VSD_SMI_CH_LINK_MOVE:
        ret = vsd_pipe_readn (pipefd, &msg->u.lnkmv_body.dst_vsys, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (LNKMV:dst_vsys) data from pipe!\n");
            return -1;
        }
        
        ret = vsd_pipe_recv_str (pipefd, &msg->u.lnkmv_body.link_name);
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (LNKMV:link_name) data from pipe!\n");
            return -1;
        }
        
        break;

      case __VSD_SMI_CH_ARGCnV:
        ret = vsd_pipe_readn (pipefd, &msg->u.exec_body.argc, sizeof (int));
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (EXEC:argc) data from pipe!\n");
            return -1;
        }
        
        msg->u.exec_body.argv = malloc ((msg->u.exec_body.argc + 1) * sizeof (char *));
        if ( msg->u.exec_body.argv == NULL ) {
            printf ("vsys error: cannot alloc body (EXEC:argv) data!\n");
            return -1;
        }
        msg->u.exec_body.argv[msg->u.exec_body.argc] = NULL;
        
        for ( i = 0; i < msg->u.exec_body.argc; i++ ) {
            ret = vsd_pipe_recv_str (pipefd, &msg->u.exec_body.argv[i]);
            if ( ret < 0 ) {
                printf ("vsys error: cannot read body (EXEC:argv[%d]) data from socket!\n", i);
                return -1;
            }
        }
        break;

      case __VSD_SMI_CH_RESPN:
        ret = vsd_pipe_readn (pipefd, &msg->u.respn_body, sizeof (struct vsd_respn_msg_body));
        if ( ret < 0 ) {
            printf ("vsys error: cannot read body (RESPN) data from socket!\n");
            return -1;
        }
        break;

      default:
        break;
    }
    return 0;
}



