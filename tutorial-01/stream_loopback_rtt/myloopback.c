#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "time.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>
#include <sched.h>
#include <stdbool.h>
#include <pthread.h>

double RTT_start;
double RTT_end;
double RTT_execution_time;
double getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}

#define N (2048*2048)

int fdr32, fdw32;
int rc;

void *rtt(void *ptr);
int main(int argc, char *argv[]) {

	pthread_t rttThread;
	char *message1 = "RTT Thread";

	int seed = time(NULL);
	srand(seed);

	char *bufcw, *bufr;
	int d_in = 326;
	int d_out = 0;
	int status;


	fdw32 = open("/dev/xillybus_write_stream", O_WRONLY);
	if ((fdw32 < 0)) {
		perror("Failed to open xillybus_write_stream Xillybus device file(s)");
		exit(1);
	}

	fdr32 = open("/dev/xillybus_read_stream", O_RDONLY);
	if ((fdr32 < 0)) {
		perror("Failed to open xillybus_read_stream Xillybus device file(s)");
		exit(1);
	}
	//=====================================================================

	int iret = pthread_create( &rttThread, NULL, rtt, (void*) message1);

	pthread_join( rttThread, NULL);

	return 0;

}


void *rtt(void *ptr) {

	pthread_t thId = pthread_self();
	pthread_attr_t thAttr;

	int policy = SCHED_RR ;
	int max_prio_for_policy = 99;

	pthread_attr_init(&thAttr);
	pthread_attr_getschedpolicy(&thAttr, &policy);
	max_prio_for_policy = sched_get_priority_max(policy);

	pthread_setschedprio(thId, max_prio_for_policy);
	pthread_attr_destroy(&thAttr);

	cpu_set_t cpuset;
	int cpu = 3;

	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 2 on cpuset
	sched_setaffinity(0, sizeof(cpuset), &cpuset);

	unsigned int tmpU32DataWrite= 124;
	unsigned int tmpU32DataRead ;

	for (int i = 0; i < 20; i++) {
		RTT_start = getTimestamp();

		rc = write(fdw32, (void *) &tmpU32DataWrite, sizeof(tmpU32DataWrite));
		rc = write(fdw32, (void *) &tmpU32DataWrite, 0);
		rc = read (fdr32, (void *) &tmpU32DataRead,  sizeof(tmpU32DataRead));

		RTT_end = getTimestamp();
		RTT_execution_time = (RTT_end-RTT_start)/(1000);
		printf("stream 32 loopback RTT  %.6lf ms \n", RTT_execution_time);
	}

	if (tmpU32DataRead != tmpU32DataWrite) {
		printf("Error stream32: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32DataRead, tmpU32DataWrite);
		return 0;
	}

	printf("Data stream32: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32DataRead, tmpU32DataWrite);

	return 0;
}
