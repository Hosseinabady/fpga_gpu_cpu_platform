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



double getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}

#define N (2048*2048)

int fdr32, fdw32;
int rc;


int main(int argc, char *argv[]) {

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
		double data_transfer_start;
		double data_transfer_end;
		double data_transfer_execution_time;
		data_transfer_start = getTimestamp();
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
		data_transfer_end = getTimestamp();
		data_transfer_execution_time = (data_transfer_end-data_transfer_start)/(1000);
		printf("**********stream 32 write loopback execution time  %.6lf ms elapsed\n", data_transfer_execution_time);

		return 1;
	} else {
		double data_transfer_start;
		double data_transfer_end;
		double data_transfer_execution_time;
		data_transfer_start = getTimestamp();
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
		data_transfer_end = getTimestamp();
		data_transfer_execution_time = (data_transfer_end-data_transfer_start)/(1000);
		printf("&&&&&&&&&stream 32 read loopback execution time  %.6lf ms elapsed\n", data_transfer_execution_time);

	}



	printf("\n");
	for (int i = 0; i < N; i++) {
		if (tmpU32ArrayDataRead[i] != tmpU32ArrayDataWrite[i]) {
			printf("Error stream32: at %d tmpDataRead = %d tmpDataWrite = %d\n", i, tmpU32ArrayDataRead[i], tmpU32ArrayDataWrite[i]);
			return -1;
		}
	}
	printf("\n");
	printf("Data stream32: tmpDataRead = %d tmpDataWrite = %d\n", tmpU32ArrayDataRead[1], tmpU32ArrayDataWrite[1]);



	return 0;

}
