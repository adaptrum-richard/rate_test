#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <memory.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <errno.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#define TOTAL_TIME (10*1000*1000) 
//#define TEST
struct parameters
{
    int packet_len;
    int interval;
    int counts;
    unsigned char dmac[6];
    unsigned char smac[6];
    int ifindex;
    int sock;
};
struct parameters arg;

static show_parameters(struct parameters *itp)
{
    printf("packet lenght:\t%d\n", itp->packet_len);
    printf("interval(us):\t%d\n", itp->interval);
    printf("counts:\t\t%d\n", itp->counts);
    unsigned char *mac = itp->dmac;
    printf("dest mac:\t%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    mac = itp->smac;
    printf("src mac:\t%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("ifindex:\t%d\n", itp->ifindex);
}

void parse_mac_address(const char *src, char *mac)
{
    sscanf(src, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}

void get_local_mac(int *ifindex, char *smac, char *ifname)
{
#define MAXINTERFACES 16
    int fd, interface;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        int i = 0;
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
        {
            interface = ifc.ifc_len / sizeof(struct ifreq);
            //printf("interface num is %d\n", interface);
            while (i < interface)
            {
                //printf("net device %s\n", buf[i].ifr_name);
                if (strcmp(buf[i].ifr_name, ifname) == 0)
                {
                    if (!(ioctl(fd, SIOCGIFHWADDR, (char *)&buf[i])))
                    {
                        memcpy(smac, buf[i].ifr_hwaddr.sa_data, 6);
                        *ifindex = i + 1;
                        break;
                    }
                }
                i++;
            }
        }
        close(fd);
    }
}

void sleep_us(long usec)
{
    struct timeval time;
    if (usec == 0)
        return;
    time.tv_sec = 0;
    time.tv_usec = usec;
    select(0, NULL, NULL, NULL, &time);
}

long get_usec()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000*1000 + t.tv_usec;
}

static void *tx(void *data)
{
    struct parameters *parameters = (struct parameters *)data;
    struct sockaddr_ll client;
    socklen_t addr_length = sizeof(struct sockaddr_ll);
    unsigned char *sendbuf = (unsigned char *)malloc(sizeof(unsigned char) * 4096);
    memset(sendbuf, 0xff, 4096);
    unsigned short family = 0x0000;
    unsigned short type = 0xCCCC;

    client.sll_family = family;
    client.sll_protocol = type;
    client.sll_ifindex = parameters->ifindex;
    client.sll_hatype = 1;
    client.sll_pkttype = 0;
    client.sll_halen = 6;
    memcpy(client.sll_addr, parameters->dmac, 6);
    unsigned char *p = sendbuf;
    //copy dmac
    memcpy(p, parameters->dmac, 6);
    p += 6;
    //copy smac
    memcpy(p, parameters->smac, 6);
    p += 6;
    //copy type
    memcpy(p, (unsigned char *)(&type), 2);
    p += 2;
    unsigned long next = 0;
    memset(p, 0, sizeof(unsigned long));
    printf("run client\n");
    int i;
    int n;
    long start = 0;
    unsigned long count = 0;
    sleep_us(10000);
#ifdef TEST
    for(int j = 0; j < 10; j++)
#else
    while (1)
#endif
    {

        if (parameters->counts != 0 && i == parameters->counts)
        {
            sleep_us(parameters->interval);
            i = 0;
        }

        //send packet
        n = sendto(parameters->sock, sendbuf, 14 + parameters->packet_len, 0, (struct sockaddr *)&client, sizeof(client));
        count += n;
        // calc rate
        if (start == 0)
            start = get_usec();
        else
        {
            long t = get_usec() - start;
            if (t >= TOTAL_TIME)
            {
                printf("tx info:\n");
                printf("\ttx bytes:\t%lu\n\ttx times:\t%ld us\n", count, t);
                printf("\ttx rate:\t%lu Mbps\n", (8 * count) / 1024 / 1024 / (t/1000/1000));
                printf("\ttx pkt sn:\t%lu\n\n", next);
                start = 0;
                count = 0;
            }
        }
#ifdef TEST
        printf("tx pkt sn: %llu\n", next);
        printf("tx pkt size = %d\n", n);
#endif
        i++;
        next++;
        memcpy(p, (unsigned char *)&next, sizeof(unsigned long));
    }
}

int init_socket()
{
    int sock, n;
    char buffer[2048];

    if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        perror("socket");
        exit(1);
    }
    return sock;
}

static void *rx(void *data)
{
    struct parameters *parameters = (struct parameters *)data;
    unsigned char *rxbuff = (unsigned char *)malloc(sizeof(char) * 4096);
    memset(rxbuff, 0xff, 4096);
    unsigned char *p = rxbuff + 14;
    unsigned long pkt_num = 0;
    unsigned long pkt_expect = 0;
    unsigned long lost_pkt_count = 0;
    unsigned short type = 0xCCCC;
    struct sockaddr_ll client;
    socklen_t addr_length = sizeof(struct sockaddr_ll);
    int n;
    long start = 0;
    long count = 0;
    while (1)
    {
        memset(rxbuff, 0x00, 4096);
        n = recvfrom(parameters->sock, rxbuff, 4096, 0, (struct sockaddr *)&client, &addr_length);
        if (n < 42)
            continue;
        if (client.sll_protocol == type)
        {
            count += n;
            memcpy((char *)&pkt_num, p, sizeof(unsigned long));
#ifdef TEST
            printf("rx pkt sn: %lu\n", pkt_num);
            printf("rx pkt size = %d\n", n);
#endif
            
            if(pkt_expect != pkt_num){
                if(pkt_expect > pkt_num){
                    lost_pkt_count += (pkt_expect - pkt_num);
                }
                else{
                    lost_pkt_count += (pkt_num - pkt_expect);
                }
                pkt_expect = pkt_num + 1;
            }else
                pkt_expect++;
            if (start == 0)
                start = get_usec();
            else
            {
                long t = get_usec() - start;
                if (t >= TOTAL_TIME)
                {
                    printf("rx info:\n");
                    printf("\tlost pkts:\t%lu\n\trx bytes:\t%lu\n\trx times:\t%ld us\n",lost_pkt_count, count, t);
                    unsigned long rate = (8 * count) / 1024 / 1024 / (t/1000/1000);
                    printf("\trx rate:\t%lu Mbps\n", rate);
                    printf("\trx pkt sn:\t%lu\n", pkt_num);
                    float packet_loss = (1.0*lost_pkt_count)/(1.0*pkt_num);
                    printf("\tpacket loss:\t%2.f%%\n", packet_loss*100);
                    printf("\trx_rate/(1-packet_lost):\t%2.f Mbps\n\n", (1.0*rate)/(1.0 - packet_loss));
                    start = 0;
                    count = 0;
                }
            }
        }
    }
}

int create_pthread_tx_rx()
{
    pthread_t tx_pid;
    pthread_t rx_pid;
    if (pthread_create(&tx_pid, NULL, tx, (void *)&arg) == -1)
    {
        printf("create tx thread failed\n");
        return -1;
    }
    if (pthread_create(&rx_pid, NULL, rx, (void *)&arg) == -1)
    {
        printf("create rx thread failed\n");
        return -1;
    }
    if (pthread_join(tx_pid, NULL))
    {
        printf("tx_pid not exit\n");
        return -1;
    }
    if (pthread_join(rx_pid, NULL))
    {
        printf("rx_pid not exit\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int opt, flags;
    /*  -l packet lenght, 
        -t interval, 
        -c Number of packets sent each time, 
        -d dest mac
        -i local send packet interface name*/
    char *optstring = "l:t:c:d:i:";
    char *dmac = NULL;
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 'l':
            arg.packet_len = atoi(optarg);
            break;
        case 't':
            arg.interval = atoi(optarg);
            break;
        case 'c':
            arg.counts = atoi(optarg);
            break;
        case 'd':
            parse_mac_address(optarg, arg.dmac);
            break;
        case 'i':
            get_local_mac(&arg.ifindex, arg.smac, optarg);
            break;
        default:
            break;
        }
    }
    show_parameters(&arg);
    arg.sock = init_socket();
    create_pthread_tx_rx();
    return 0;
}