

#include "command.h"


double hardware_start;
double hardware_end;
double hardware_execution_time;
double getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}

#define N (2048*2048/4)
int fdracknowledge, fdwcommand;
int fdr32_1, fdw32_1;
int fdr32_2, fdw32_2;

int rc;


int main(int argc, char *argv[]) {

	printf("Hello scaler\n");

	int seed = time(NULL);
	srand(seed);

	char *bufcw, *bufr;
	int d_in = 326;
	int d_out = 0;
	int status;



	fdwcommand = open("/dev/xillybus_command", O_WRONLY );
	if ((fdwcommand < 0)) {
		perror("Failed to open xillybus_stream_in Xillybus device file(s)");
		exit(1);
	}

	fdracknowledge = open("/dev/xillybus_acknowledge", O_RDONLY );
	if ((fdracknowledge < 0)) {
		perror("Failed to open xillybus_stream_out Xillybus device file(s)");
		exit(1);
	}

	fdw32_1 = open("/dev/xillybus_write_01_32", O_WRONLY);
	if ((fdw32_1 < 0)) {
		perror("Failed to open xillybus_stream_in Xillybus device file(s)");
		exit(1);
	}

	fdr32_1 = open("/dev/xillybus_read_01_32", O_RDONLY );
	if ((fdr32_1 < 0)) {
		perror("Failed to open xillybus_stream_out Xillybus device file(s)");
		exit(1);
	}

	fdw32_2 = open("/dev/xillybus_write_02_32", O_WRONLY);
	if ((fdw32_2 < 0)) {
		perror("Failed to open xillybus_stream_in Xillybus device file(s)");
		exit(1);
	}

	fdr32_2 = open("/dev/xillybus_read_02_32", O_RDONLY );
	if ((fdr32_2 < 0)) {
		perror("Failed to open xillybus_stream_out Xillybus device file(s)");
		exit(1);
	}


	//=====================================================================

	DATA_TYPE *a = (DATA_TYPE *)malloc(N*sizeof(DATA_TYPE));
	DATA_TYPE *b = (DATA_TYPE *)malloc(N*sizeof(DATA_TYPE));
	for (int i = 0; i < N; i++) {
		a[i] = rand();
		b[i] = 0;
	}




	measure_RTT();



	//----------------------------------------------------------------------
	status = fpga_init();
	//-----------------------------------------------------------
	uint32_t  *a_fpga = (uint32_t  *)fpga_alloc(N*sizeof(DATA_TYPE), IN);
	uint32_t  *b_fpga = (uint32_t  *)fpga_alloc(N*sizeof(DATA_TYPE), OUT);

	printf("a_fpga=%p\n", a_fpga);
	printf("b_fpga=%p\n", b_fpga);
	//-----------------------------------------------------------



	hardware_start = getTimestamp();


	status = fpga_memcpy(a, a_fpga, N*sizeof(DATA_TYPE), IN);


	hardware_end = getTimestamp();
	hardware_execution_time = (hardware_end-hardware_start)/(1000);
	printf("&&&&&&&&& fpga_memcpy write execution time  %.6lf ms elapsed\n", hardware_execution_time);

	//-----------------------------------------------------------


	hardware_start = getTimestamp();

	accelerateRun();


	hardware_end = getTimestamp();
	hardware_execution_time = (hardware_end-hardware_start)/(1000);
	printf("&&&&&&&&& accel  execution time  %.6lf ms elapsed\n", hardware_execution_time);

	//-----------------------------------------------------------


	hardware_start = getTimestamp();

	printf("b=0x%x\n", b);
	status = fpga_memcpy(b_fpga, b, N*sizeof(DATA_TYPE), OUT);

	hardware_end = getTimestamp();
	hardware_execution_time = (hardware_end-hardware_start)/(1000);
	printf("&&&&&&&&& fpga_memcpy read execution time  %.6lf ms elapsed\n", hardware_execution_time);



	//check data
	for (int i = 0;i < N; i++) {
		if (b[i] != 2*a[i]+1) {
			printf("Error in b[%d]=%u != a[%d]=%u\n", i, b[i], i, 2*a[i]+1);
			if (i > 10 )
				break;
		}
		if (i < 10 )
			printf("Data in b[%d]=%u = a[%d]=%u\n", i, b[i], i, 2*a[i]+1);
	}



	printf("Data check successful\n");

	printf("Bye scaler\n");
	return 0;
}




