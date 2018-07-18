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

double hardware_start;
double hardware_end;
double hardware_execution_time;
double getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}

#define N (2048*2048)
int fdr8, fdw8;
int fdr32, fdw32;
int fdwcommand, fdrstatus;
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

	fdw8 = open("/dev/xillybus_write_8", O_WRONLY);
	if ((fdw8 < 0)) {
		perror("Failed to open xillybus_stream_in Xillybus device file(s)");
		exit(1);
	}

	fdr8 = open("/dev/xillybus_read_8", O_RDONLY);
	if ((fdr8 < 0)) {
		perror("Failed to open xillybus_stream_out Xillybus device file(s)");
		exit(1);
	}

	fdw32 = open("/dev/xillybus_write_32", O_WRONLY);
	if ((fdw32 < 0)) {
		perror("Failed to open xillybus_stream_in Xillybus device file(s)");
		exit(1);
	}

	fdr32 = open("/dev/xillybus_read_32", O_RDONLY);
	if ((fdr32 < 0)) {
		perror("Failed to open xillybus_stream_out Xillybus device file(s)");
		exit(1);
	}
	fdwcommand = open("/dev/xillybus_send_command", O_WRONLY);
	if ((fdwcommand < 0)) {
		perror("Failed to open xillybus_send_command Xillybus device file(s)");
		exit(1);
	}

	fdrstatus = open("/dev/xillybus_receive_status", O_RDONLY);
	if ((fdrstatus < 0)) {
		perror("Failed to open xillybus_receive_status Xillybus device file(s)");
		exit(1);
	}
	//=====================================================================



/*
	unsigned char tmpCharDataWrite = 23;
	unsigned char tmpCharDataRead = 23;

	hardware_start = getTimestamp();
	rc = write(fdw8, (void *) &tmpCharDataWrite, sizeof(tmpCharDataWrite));
	rc = write(fdw8, (void *) &tmpCharDataWrite, 0);
	rc = read (fdr8, (void *) &tmpCharDataRead,  sizeof(tmpCharDataRead));
	hardware_end = getTimestamp();
	hardware_execution_time = (hardware_end-hardware_start)/(1000);
	printf("=========stream 8 read loopback execution time  %.6lf ms elapsed\n", hardware_execution_time);

	if (tmpCharDataRead != tmpCharDataWrite) {
		printf("Error stream8: tmpDataRead = %d tmpDataWrite = %d\n", tmpCharDataRead, tmpCharDataWrite);
		return -1;
	}

	printf("Data stream8: tmpCharDataRead = %d tmpCharDataWrite = %d\n", tmpCharDataRead, tmpCharDataWrite);
*/

	//-----------------------------------------------------------------------------------------------------------

	int iret = pthread_create( &rttThread, NULL, rtt, (void*) message1);


	pthread_join( rttThread, NULL);

/*
	unsigned int *tmpU32ArrayDataWrite = (unsigned int *)malloc(sizeof(unsigned int )*N);
	unsigned int *tmpU32ArrayDataRead = (unsigned int *)malloc(sizeof(unsigned int )*N);

	for (int i = 0; i < N; i++) {
		tmpU32ArrayDataWrite[i] = rand();
	}

	//rc = write(fdw32, (void *) tmpU32DataWrite, N*sizeof(unsigned int));

	int donebytes = 0;
	char* bufWrite = (char *) tmpU32ArrayDataWrite;
	char* bufRead = (char *) tmpU32ArrayDataRead;

	pid_t pid = fork();
	if (pid < 0) {
		perror("Failed to fork()");
		exit(1);
	}
	if (pid) {
		double hardware_start;
		double hardware_end;
		double hardware_execution_time;
		hardware_start = getTimestamp();
		while (donebytes < sizeof(unsigned int) * N) {
			rc = write(fdw32, bufWrite + donebytes,
					sizeof(unsigned int) * N - donebytes);
			if ((rc < 0) && (errno == EINTR))
				continue;
			if (rc <= 0) {
				perror("write() failed");
				exit(1);
			}
			donebytes += rc;
		}
		hardware_end = getTimestamp();
		hardware_execution_time = (hardware_end-hardware_start)/(1000);
		printf("**********stream 32 write loopback execution time  %.6lf ms elapsed\n", hardware_execution_time);
	} else {
		double hardware_start;
		double hardware_end;
		double hardware_execution_time;
		hardware_start = getTimestamp();
		donebytes = 0;
		while (donebytes < sizeof(unsigned int) * N) {
			rc = read(fdr32, bufRead + donebytes,
					sizeof(unsigned int) * N - donebytes);
			if ((rc < 0) && (errno == EINTR))
				continue;
			if (rc < 0) {
				perror("read() failed");
				exit(1);
			}
			if (rc == 0) {
				fprintf(stderr, "Reached read EOF!? Should never happen.\n");
				exit(0);
			}
			donebytes += rc;
		}
		hardware_end = getTimestamp();
		hardware_execution_time = (hardware_end-hardware_start)/(1000);
		printf("&&&&&&&&&stream 32 read loopback execution time  %.6lf ms elapsed\n", hardware_execution_time);
		return 1;
	}
	//rc = read (fdr32, (void *) tmpU32DataRead,  N*sizeof(unsigned int));

	sleep(1);

	for (int i = 0; i < N; i++) {
		if (tmpU32ArrayDataRead[i] != tmpU32ArrayDataWrite[i]) {
			printf("Error stream32: at %d tmpDataRead = %d tmpDataWrite = %d\n", i, tmpU32ArrayDataRead[i], tmpU32ArrayDataWrite[i]);
			return -1;
		}
	}
	printf("Data stream32: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32ArrayDataRead[1], tmpU32ArrayDataWrite[1]);

*/
/*
	tmpU32DataWrite= 524;
	tmpU32DataRead ;

	hardware_start = getTimestamp();
	rc = write(fdwcommand, (void *) &tmpU32DataWrite, sizeof(tmpU32DataWrite));
	rc = read (fdrstatus, (void *) &tmpU32DataRead,  sizeof(tmpU32DataRead));
	hardware_end = getTimestamp();
	hardware_execution_time = (hardware_end-hardware_start)/(1000);
	printf("=====command-status 23 loopback execution time  %.6lf ms elapsed\n", hardware_execution_time);

	if (tmpU32DataRead != tmpU32DataWrite) {
		printf("Error command-status: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32DataRead, tmpU32DataWrite);
		return -1;
	}

	printf("Data command-status: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32DataRead, tmpU32DataWrite);
*/

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

	for (int i = 0; i < 1; i++) {
		hardware_start = getTimestamp();

		rc = write(fdw32, (void *) &tmpU32DataWrite, sizeof(tmpU32DataWrite));
		rc = write(fdw32, (void *) &tmpU32DataWrite, 0);
		rc = read (fdr32, (void *) &tmpU32DataRead,  sizeof(tmpU32DataRead));

		hardware_end = getTimestamp();
		hardware_execution_time = (hardware_end-hardware_start)/(1000);
		printf("stream 32 loopback RTT  %.6lf ms \n", hardware_execution_time);
	}

	if (tmpU32DataRead != tmpU32DataWrite) {
		printf("Error stream32: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32DataRead, tmpU32DataWrite);
		return 0;
	}

	printf("Data stream32: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32DataRead, tmpU32DataWrite);

	return 0;
}
