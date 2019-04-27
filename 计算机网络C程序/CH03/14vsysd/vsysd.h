#ifndef __VSYSD_H__
#define __VSYSD_H__

enum vsd_message_command {
    VSD_MSGCMD_CREATE = 1,
    VSD_MSGCMD_DESTROY = 2,
    VSD_MSGCMD_RUN = 3,
    VSD_MSGCMD_LNK_MOVE = 4,
    VSD_MSGCMD_RESPONSE = 5
};

struct vsd_message_head {
    unsigned char magic_field[4];
    char command;
    unsigned char vsysid;
    unsigned short reserved;
};

struct vsd_exec_msg_body {
    int argc;
    char **argv;
};

struct vsd_link_move_msg_body {
    char *link_name;
    unsigned char dst_vsys;
};

enum vsd_response_type {
    VSD_RESPN_VSYS_CREATE_SUCCESS,
    VSD_RESPN_VSYS_CREATE_FAILED,
    VSD_RESPN_VSYS_DESTROY_SUCCESS,
    VSD_RESPN_VSYS_DESTROY_FAILED,
    VSD_RESPN_VSYS_EXEC_START_SUCCESS,
    VSD_RESPN_VSYS_EXEC_START_FAILED,
    VSD_RESPN_VSYS_EXEC_FINISHED,
    VSD_RESPN_VSYS_EXEC_TERMINATED,
    VSD_RESPN_VSYS_LINK_MOVED,
    VSD_RESPN_VSYS_LINK_NOT_MOVED,
    VSD_RESPN_VSYS_NOT_EXIST,
    VSD_RESPN_VSYS_EXISTED,
    VSD_RESPN_LINK_NOT_EXIST,
    VSD_RESPN_EXEC_NOT_EXIST,
    VSD_RESPN_COMMAND_UNKNOWN
};

struct vsd_respn_msg_body {
    unsigned char command;
    unsigned char reserved;
    short respn_type;
};

struct vsd_message {
    struct vsd_message_head head;
    union {
        struct vsd_exec_msg_body exec_body;
        struct vsd_link_move_msg_body lnkmv_body;
        struct vsd_respn_msg_body respn_body;
    } u;
};

enum vsd_vsys_state_type {
    VSD_STAT_NULL = 0,
    VSD_STAT_CREATING = 1,
    VSD_STAT_READY = 16,
    VSD_STAT_DEADING = 3
};

#define VSD_VSYS_MAX_NUM  256
#define VSD_USR_DOMSK_PATH "/var/run/vsysd.sock"

extern int vsd_user_dom_sock;

static inline int 
vsd_is_valid_vsys_id (int vsysid)
{
    return (vsysid >= 0 && vsysid < VSD_VSYS_MAX_NUM);
}

int vsd_get_cmdtype_num ();
int vsd_parse_cmdtype_str (char *cmdstr);
const char *vsd_get_cmdtype_str (int cmdtype);
int vsd_parse_vsys_str (char *vsysstr);

int vsd_make_message_magic_field (struct vsd_message_head *msghead);
int vsd_check_message_magic_field (struct vsd_message_head *msghead);

int vsd_check_response_msg (int command, struct vsd_message *msg);
void vsd_clean_message_heap_mem (struct vsd_message *msg);
void vsd_dump_message (struct vsd_message *msg);

int vsd_sock_send_message(int sock, struct vsd_message *msg);
int vsd_sock_recv_message(int sock, struct vsd_message *msg);

int vsd_pipe_send_message(int pipefd, struct vsd_message *msg);
int vsd_pipe_recv_message(int pipefd, struct vsd_message *msg);




int vsd_send_vsys_message(struct vsd_message *msg);
int vsd_recv_vsys_message(struct vsd_message *msg);
int vsd_recv_user_message(struct vsd_message *msg);


#endif

