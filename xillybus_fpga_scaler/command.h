#ifndef __COMMAND_C__
#define __COMMAND_C__
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
#include <stdint.h>


enum fpga_accel_operation_enum{
	FPGAMEMORY_OK=10,
	INIT_FPGA,
	MALLOC,
	WRITE_TO_FPGADDR3,
	READ_FROM_FPGADDR3,
	ACCELERATOR_ARG,
	ACCELERATOR_START,
	ALLOCATION_ERROR,
	WRITE_TO_FPGADDR3_ERROR,
	LOOP_BACK
};
typedef enum fpga_accel_operation_enum fpga_accel_operation_type;


enum fpga_argument_enum{SCALAR=10, POINTER, ARRAY};
typedef enum fpga_argument_enum fpga_argument_type;

enum fpga_argument_direction_enum{IN=10, OUT, INOUT};
typedef enum fpga_argument_direction_enum fpga_argument_direction_type;

#define DATA_TYPE uint32_t
struct fpga_accel_command_struct {
	uint32_t   command_id;         //{0: none, 1: malloc
	uint32_t   variable_address;
	uint32_t   variable_index;
	uint32_t   variable_type;
	uint32_t   variable_direction;
	uint32_t   variable_data;
	uint32_t   variable_data_size;
};
typedef struct fpga_accel_command_struct fpga_accel_command;


int sendCommand( fpga_accel_command *command);
int receiveAcknowledge(fpga_accel_command *acknowledge);
int WriteData2FPGADDR3( fpga_accel_command command, void *data);
int ReadDataFromFPGADDR3(fpga_accel_command command, void *data);

int fpga_init();
void* fpga_alloc(uint32_t size, fpga_argument_direction_type direction);
int fpga_memcpy(void *src, void *dst, uint32_t size, fpga_argument_direction_type direction );
int setArg(uint32_t size, fpga_argument_direction_type direction);
int accelerateRun();
int measure_RTT();
#endif//__COMMAND_C__
