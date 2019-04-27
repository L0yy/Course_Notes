#include <stdio.h>
#include <sys/types.h>       // socket
#include <sys/socket.h>      // socket
#include <sys/ioctl.h>       // ioctl
#include <net/if.h>          // ifreq
#include <netinet/if_ether.h>
#include <sys/types.h>
#include <string.h>          // strcpy
#include <linux/if_packet.h> // sockaddr_ll
#include <linux/filter.h>
#include <arpa/inet.h>

#define BOOL   unsigned char
#define U8     unsigned char
#define U16    unsigned short
#define TRUE   1
#define FALSE  0

static char if_name[32] = "eth0";
static int sock;
static unsigned char if_mac[6];
static int if_index;

uint8_t g_buf[2048] = { 0 };
uint8_t g_txbuf[2048] = { 0 };
uint16_t g_len = 0;

uint8_t ALL_L1_ISS[6] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x14 };
uint8_t ALL_L2_ISS[6] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x15 };
uint8_t ALL_ISS[6] = { 0x09, 0x00, 0x2B, 0x00, 0x00, 0x05 };
uint8_t ALL_ESS[6] = { 0x09, 0x00, 0x2B, 0x00, 0x00, 0x04 };

struct sock_filter bpf_code[] = {
    { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 1, 0x000086dd },
    { 0x06, 0, 0, 0x0000ffff },
    { 0x06, 0, 0, 0x00000000 },
};

struct sock_fprog filter = {
	.len = sizeof (bpf_code) / sizeof(struct sock_filter),
	.filter = bpf_code
};

static int
init_ether_if (void)
{
    struct ifreq req;
    struct sockaddr_ll sl;

    sock = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
    if ( sock < 0 ) {
        printf ("failed to create raw socket!\n");
        return -1;
    }

    strcpy (req.ifr_name, if_name);
    if ( ioctl (sock, SIOCGIFFLAGS, &req ) < 0 ) {
       printf ("failed to do ioctl!");
       return -1;
    }

    printf ("IF Name: %s\n", req.ifr_name);
    printf ("IF Flags: 0x%08x\n", req.ifr_flags);
    req.ifr_flags |= IFF_PROMISC;

    if ( ioctl (sock, SIOCSIFFLAGS, &req) < 0 ) {
        printf ("failed to set eth0 into promisc mode!");
        return -1;
    }

    if ( ioctl (sock, SIOCGIFHWADDR, &req) < 0 ) {
        printf ("failed to get IF hw address!");
        return -1;
    }

    memcpy (if_mac, req.ifr_hwaddr.sa_data, sizeof(if_mac));
    printf ("IF HwAddr: %02x:%02x%02x:%02x:%02x%02x\n",
            if_mac[0], if_mac[1], if_mac[2], if_mac[3], if_mac[4], if_mac[5]);

    if ( ioctl (sock, SIOCGIFINDEX, &req) < 0 ) {
        printf ("failed to get IF hw address!");
        return -1;
    }
    if_index = req.ifr_ifindex;
    printf ("IF Idx: %d\n", if_index);

    memset (&sl, 0x00, sizeof(sl));
    sl.sll_family = AF_PACKET;
    sl.sll_protocol = htons (0x0003);
    sl.sll_ifindex = if_index;

    //if ( bind (sock, (struct sockaddr *)&sl, sizeof (sl)) < 0 ) {
    //    printf( "failed to bind!\n" );
    //    return -1;
    //}

    //if ( setsockopt (sock, SOL_SOCKET, SO_ATTACH_FILTER,
    //                 &filter, sizeof (filter)) < 0) {
    //    printf("failed to add filter\n");
    //}

    return 0;
}

static BOOL
recv_package (uint8_t *buf, U16 *len)
{
    int retlen = 0;

    retlen = recvfrom (sock, buf, 2000, 0, NULL, NULL);
    if ( retlen < 0 )
        return -1;

    if ( len != NULL )
        *len = retlen;
    return 0;
}

static BOOL
send_package (U8 *buf, U16 len)
{
    struct sockaddr_ll sl;

    sl.sll_ifindex = if_index;
    memcpy (sl.sll_addr, buf, sl.sll_halen);

    if ( sendto (sock, buf, len, 0,
                 (struct sockaddr *)&sl, sizeof (sl)) < 0 ) {
        printf ("failed to send to RAW socket!\n");
        return -1;
   }
   return 0;
}

static BOOL
fill_mac_addr (uint8_t *mac)
{
    memcpy (mac, if_mac, sizeof (if_mac));
    return TRUE;
}

static void
dump_buf (uint8_t *buf, uint16_t size)
{
    static int cnt = 0;
    int i;

    cnt++;

    //if ( size < 6 )
    //    return;
    //if ( buf[0] != 0x01 || buf[1] != 0x80 || buf[2] != 0xc2 )
    //     return;

    printf ("[%6d] Recv an IPv6 packet. Length = %u", cnt, (unsigned)size);

    if ( size > 64 )
        size = 64;
    for ( i = 0; i < size; i++ ) {
        if ( (i % 16) == 0 )
            printf ("\n            ");
        printf ("%02x ",buf[i]);
    }
    printf ("\n");
}

int main (int argc, char *argv[])
{
    BOOL ret;
    char *p;

    if ( argc > 1 ) {
        strncpy (if_name, argv[1], 31);
    }

    memcpy (g_txbuf, ALL_L1_ISS, 6);

    g_txbuf[6 + 6] = 0x00;
    g_txbuf[6 + 6 + 1] = 64 + 6 + 6 + 2;
    for ( ret = 0; ret < 64; ret++ )
        g_txbuf[ret + 6 + 6 + 2] = ret;

    if ( init_ether_if () < 0 )
        return 1;

    printf ("send:\n");
    fill_mac_addr (g_txbuf + 6);
    dump_buf (g_txbuf, 64);
    send_package (g_txbuf, 64);

    while ( 1 ) {
        if ( recv_package (g_buf, &g_len) < 0 )
            continue;

        dump_buf(g_buf,g_len);

        memcpy (g_txbuf, g_buf, g_len);
        memcpy (g_txbuf, g_buf + 6, 6);
        p = g_txbuf + 6;
        fill_mac_addr (p);
        *(g_txbuf + 11) = *(g_txbuf + 11) + 1;
        send_package (g_txbuf, g_len);
    }
    return 0;
}
