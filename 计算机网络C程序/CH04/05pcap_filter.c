#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap/pcap.h>

int main (int argc, char *argv[])
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *ph;
    struct bpf_program bpfprog;
    char filterbuf[64] = { '\0' };
    struct pcap_pkthdr *pkthdr;
    const u_char *pktdat;
    pcap_dumper_t *pf;
    int i;

    if ( argc < 4 )
        return 0;
    for (i = 3; i < argc; i++) {
        strcat (filterbuf, argv[i]);
        strcat (filterbuf, " ");
    }

    ph = pcap_open_offline (argv[1], errbuf);
    if ( ph == NULL ) {
        printf ("ERROR: Cannot open PCAP file.\n");
        return 1;
    }

    pf = pcap_dump_open (ph, argv[2]);
    
    pcap_compile (ph, &bpfprog, filterbuf, 1, PCAP_NETMASK_UNKNOWN);
    pcap_setfilter (ph, &bpfprog);

    while ( pcap_next_ex (ph, &pkthdr, &pktdat) >= 0 ) {
        pcap_dump ((u_char *)pcap_dump_file (pf), pkthdr, pktdat);
    }

    //while ( pcap_next_ex(ph, &pkthdr, &pktdat) >= 0 ) {
    //    if (pcap_offline_filter (&bpfprog, pkthdr, pktdat)) {
    //        pcap_dump ((u_char *)pcap_dump_file(pf), pkthdr, pktdat);
    //    }
    //}

    pcap_dump_flush (pf);
    pcap_dump_close (pf);

    pcap_freecode (&bpfprog);
    pcap_close (ph);
    return 0;
}
