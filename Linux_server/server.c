#include <string.h>
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include "hash_func.h"
#include "epoll_helper.h"
#include "assert.h"
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <sched.h>
#include <sys/types.h>
#include <ctype.h>

#define MAX_LEN 1024
#define HI_STIME 5000
#define LO_STIME 20
#define DL_HI 500*1000
#define DL_LO 5*1000
#define __NR_sched_setattr 314
#define __NR_sched_getattr 315

static int numget = 1;
int reqcnt[2][65536];

struct sched_attr {
	uint32_t size;
	uint32_t sched_policy;
	uint64_t sched_flag;
	int32_t  sched_nice;
	uint32_t sched_priority;
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

typedef struct udphdr_s {
	uint16_t rqid;
	uint16_t partno;
	uint16_t nparts;
	uint16_t reserved;
} udphdr_t;

struct port2pid *p2p = NULL;
struct fd2port  *f2p = NULL;
volatile unsigned long long it;
volatile unsigned long long it_inf;
unsigned long long hash_mask = ~(unsigned long long)1 >> 32;
int chain_len = 4;
int server_port = 0;

int
sched_setattr (pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

int
sched_getattr (pid_t pid, const struct sched_attr *attr, unsigned int size, unsigned int flags)
{
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

static inline unsigned long long
mb_tsc(void) {
	unsigned long a, d, c;

	__asm__ __volatile__("rdtsc" : "=a" (a), "=d" (d), "=c" (c) : : );

	return ((unsigned long long)d << 32) | (unsigned long long)a;
}

void
print_recv(char *recv, int len)
{
	int i = 0;

	for (i= 0; i < len ; i++) {
		printf("%x ",recv[i]);
	}
	printf("\n");
}

static inline void
spin_delay(unsigned long long loop)
{
	while (it < loop) {
		it ++;
	}
	it = 0;
	return;
}

static long
get_us_interval(struct timeval *start, struct timeval *end)
{
	return (((end->tv_sec - start->tv_sec) * 1000000)
		+ (end->tv_usec - start->tv_usec));
}

unsigned long long
get_cpufreq(void)
{
	struct timeval start;
	struct timeval end;
	unsigned long long tstart;
	unsigned long long tend;
	long usec;

	if (gettimeofday(&start, 0)) assert(0);
	tstart = mb_tsc();
	usleep(10000);

	if (gettimeofday(&end, 0))
		assert(0);
	tend = mb_tsc();

	usec = get_us_interval(&start, &end);
	return (tend-tstart) / (unsigned long long)usec;
}

int
is_set(void* data) {
	if (strncmp(data, "get", 3) == 0)
		return 0;
	else if (strncmp(data, "set", 3) == 0)
		return 1;
	else
		return -1;
}

int
handle_ekf(int outfd)
{
	char data[MAX_LEN];
	char *body;
	int len, nfds;
	struct sockaddr_in clientAddr;
	struct sockaddr_in memcachedAddr;
	struct epoll_event events[EPOLL_SIZE];
	int isSet = 0;
	int key;
	int cnt;

	memset(&clientAddr, 0, sizeof(clientAddr));
	clientAddr.sin_addr.s_addr = inet_addr("10.10.1.1");;
	//clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");;
	clientAddr.sin_port = htons(server_port);
	clientAddr.sin_family = AF_INET;
	socklen_t cliLen = sizeof(clientAddr);

	memset(&memcachedAddr, 0, sizeof(memcachedAddr));
	memcachedAddr.sin_addr.s_addr = inet_addr("10.10.1.2");;
	//memcachedAddr.sin_addr.s_addr = inet_addr("127.0.0.1");;
	memcachedAddr.sin_port = htons(11211);
	memcachedAddr.sin_family = AF_INET;
	socklen_t memcachedLen = sizeof(memcachedAddr);
   	
	int memsockfd = socket(AF_INET, SOCK_DGRAM, 0);

	int epfd = epoll_create1(0);
	if (epfd < 0) {
		close(memsockfd);
		close(outfd);
		assert(0);
	}

	addfd(epfd, memsockfd);
	addfd(epfd, outfd);
	while (1) {
		nfds = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if (nfds < 0) {
			close(outfd);
			close(memsockfd);
			close(epfd);
			assert(0);
		}
		for (int i = 0; i < nfds; i++) {
			if (events[i].events & EPOLLIN) {
				//printf("epoll someting\n");
				if (events[i].data.fd == outfd) {
					//printf("recvfrom out\n");
					while (1) {
					len = recvfrom(outfd, data, MAX_LEN, 0, (struct sockaddr*)&clientAddr, &cliLen);
					if (len == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
					/*printf("szie: %d, reqid: %d, seqid: %d, frag: %d, res: %d, len: %d\n",
						       	sizeof(udphdr_t), 
							ntohs(((udphdr_t*)data)->rqid), 
							ntohs(((udphdr_t*)data)->partno),
							ntohs(((udphdr_t*)data)->nparts),
							ntohs(((udphdr_t*)data)->reserved), len);*/
					//body = data+sizeof(udphdr_t);
					//aisSet = is_set(body);
					//key = atoi(body+14);
					/*printf("%s, len: %d\n", body, len);
					for (int j = 0; j < 200; j++) {
						if (j==128) printf("!!!");
						if (body[j] == '\r')
							printf("\\r");
						else if (body[j] == '\n')
							printf("\\n");
		//				else if (isprint(body[j]))
		//					printf("%c", body[j]);
						else
							printf("%02x ", (unsigned char)body[j]);
					}
					printf("\n");*/
					for (int j = 0; j < numget; j++) {
						len = sendto(memsockfd, data, len, 0, (struct sockaddr*)&memcachedAddr, memcachedLen);
						assert(len > 0);
						reqcnt[0][ntohs(((udphdr_t*)data)->rqid)] ++;
					}
					}
				}
				if (events[i].data.fd == memsockfd) {
					//printf("recvfrom memcached\n");
					while(1) {
						len = recvfrom(memsockfd, data, MAX_LEN, 0, (struct sockaddr*)&memcachedAddr, &memcachedLen);
						if (len == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
					body = data+sizeof(udphdr_t);
					printf("-");
					if (reqcnt[0][ntohs(((udphdr_t*)data)->rqid)] >= numget) {
						printf(".");
						len = sendto(outfd, data, len, 0, (struct sockaddr*)&clientAddr, cliLen);
						assert(len > 0);
						reqcnt[0][ntohs(((udphdr_t*)data)->rqid)] -= numget;
					}
					}
				}
			}
		}
	}
	return 0;
}

void
getopts(int argc, char** argv)
{
	int count = 1;

	while (count < argc) {
		if (strcmp(argv[count], "--port") == 0) {
			if (++count < argc)
				server_port = atoi(argv[count]);
		} else {
			assert(0);
		}
		count ++;
	}
}

int 
main(int argc, char *argv[]) {

	struct    sockaddr_in serverAddr;
	
	if (argc > 1)
		getopts(argc, argv);

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = PF_INET;
   	serverAddr.sin_port = htons(server_port);
   	serverAddr.sin_addr.s_addr = inet_addr("10.10.1.2");
   	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
   	int listener = socket(AF_INET, SOCK_DGRAM, 0);

   	if(listener < 0) {
		perror("listener"); 
		exit(-1);
	}
    
   	if(bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
       		perror("bind error");
       		exit(-1);
   	}
	
	handle_ekf(listener);
	assert(0);
   	close(listener); //close socket
}
