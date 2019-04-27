#include <stdio.h>
#include <stdlib.h>

#include <pcap/pcap.h>

int main (int argc, char *argv[])
{
    char errbuf[PCAP_ERRBUF_SIZE];
    char *ifname = "eth0";
    pcap_t *pcap_handle;
    pcap_dumper_t *pcap_fp;
    struct pcap_pkthdr *pkghdr;
    const u_char *pkgbuf;
    int i;

    if ( argc > 1 )
        ifname = argv[1];

    pcap_handle = pcap_create (ifname, errbuf);
    if ( pcap_handle == NULL ) {
        printf ("ERROR: Cannot create PCAP handle\n");
        return 1;
    }

    pcap_set_promisc (pcap_handle, 1);
    pcap_activate (pcap_handle);

    pcap_fp = pcap_dump_open (pcap_handle, "testdump.pcap");

    for (i = 0; i < 20; i++) {
        pcap_next_ex (pcap_handle, &pkghdr, &pkgbuf);
        pcap_dump ((u_char *)pcap_dump_file (pcap_fp), pkghdr, pkgbuf);

        printf (".");
        fflush (stdout);
    }
    pcap_dump_flush (pcap_fp);
    pcap_dump_close (pcap_fp);

    pcap_close (pcap_handle);
    return 0;
}
