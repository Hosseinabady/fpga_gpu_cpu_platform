#define _GNU_SOURCE
#include <cstdint>
#include <sched.h>
#include "command.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

extern double hardware_start;
extern double hardware_end;
extern double hardware_execution_time;
double getTimestamp();

extern int fdwcommand;
extern int fdracknowledge;

extern int fdr32_1, fdr32_2;
extern int fdw32_1, fdw32_2;

int sendCommand( fpga_accel_command *command) {


	int rc;

	rc = write(fdwcommand, &command->command_id,         sizeof(command->command_id));
	rc = write(fdwcommand, &command->variable_address,   sizeof(command->variable_address));
	rc = write(fdwcommand, &command->variable_index,     sizeof(command->variable_index));
	rc = write(fdwcommand, &command->variable_type,      sizeof(command->variable_type));
	rc = write(fdwcommand, &command->variable_direction, sizeof(command->variable_direction));
	rc = write(fdwcommand, &command->variable_data,      sizeof(command->variable_data));
	rc = write(fdwcommand, &command->variable_data_size, sizeof(command->variable_data_size));
	rc = write(fdwcommand, command, 0);

//	write(fdwcommand, command, 0);
/*
	char *buf = (char *)command;
	int donebytes = 0;
	while (donebytes <sizeof(fpga_accel_command)) {
		rc = write(fdwcommand, buf + donebytes, sizeof(fpga_accel_command) - donebytes);
		write(fdwcommand, command, 0);
		if ((rc < 0) && (errno == EINTR))
			continue;
		if (rc <= 0) {
			perror("write() failed");
			exit(1);
		}

		donebytes += rc;

	}
*/


	return 0;
}

int receiveAcknowledge( fpga_accel_command *command){


	int rc;
	rc = read(fdracknowledge, &command->command_id,         sizeof(command->command_id));
	rc = read(fdracknowledge, &command->variable_address,   sizeof(command->variable_address));
	rc = read(fdracknowledge, &command->variable_index,     sizeof(command->variable_index));
	rc = read(fdracknowledge, &command->variable_type,      sizeof(command->variable_type));
	rc = read(fdracknowledge, &command->variable_direction, sizeof(command->variable_direction));
	rc = read(fdracknowledge, &command->variable_data,      sizeof(command->variable_data));
	rc = read(fdracknowledge, &command->variable_data_size, sizeof(command->variable_data_size));

/*	char *buf = (char *)command;
	int donebytes = 0;
	while (donebytes < sizeof(fpga_accel_command)) {
		rc = read(fdracknowledge, buf + donebytes, sizeof(fpga_accel_command) - donebytes);
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

*/



	return 0;
}

/*
int WriteData2FPGADDR3( fpga_accel_command command, void *data) {


	int r;
	fpga_accel_command WriteToFPGADDR3_read_back_command;
	pid_t cpid;

	r = sendCommand(&command);
	r = receiveAcknowledge(&WriteToFPGADDR3_read_back_command);



	if (WriteToFPGADDR3_read_back_command.command_id == FPGAMEMORY_OK) {
		unsigned int datasize = command.variable_data_size / 2;

		void* data_1 = data;
		void* data_2 = data + datasize;
		int status=0;
//		printf("data_1=0x%x, data_2=0x%x, datasize= %u\n", data_1, data_2, datasize);
		pid_t   pid;
		if ((pid = fork()) < 0) {
			printf("Failed to fork process 1\n");
		    exit(1);
		} else if (pid == 0)
		{
			int rc;
			char *buf;
			buf = (char *) data_1;
			int donebytes = 0;
			while (donebytes < datasize) {
				rc = write(fdw32_1, buf + donebytes, datasize - donebytes);
				if ((rc < 0) && (errno == EINTR))
					continue;
				if (rc <= 0) {
					perror("write() failed");
					exit(1);
				}
				donebytes += rc;
			}
			exit(0);
		}
		else
		{
			int  rc;
			char *buf;
			buf = (char *) data_2;
			int donebytes = 0;
			while (donebytes < datasize) {
				rc = write(fdw32_2, buf + donebytes, datasize - donebytes);
				if ((rc < 0) && (errno == EINTR))
					continue;
				if (rc <= 0) {
					perror("write() failed");
					exit(1);
				}
				donebytes += rc;
			}
//			wait(&status);
		}
	} else {
		printf("Error in writing data to the FPGA DDR3\n");
	}

	r = receiveAcknowledge(&WriteToFPGADDR3_read_back_command);
	return 0;
}
*/


void WriteData2FPGADDR3_thread(int fdw32, uint32_t datasize, void *data, int coreid) {


	int rc;
	char *buf;
	buf = (char *) data;
	int donebytes = 0;
	while (donebytes < datasize) {
		rc = write(fdw32, buf + donebytes, datasize - donebytes);
		if ((rc < 0) && (errno == EINTR))
			continue;
		if (rc <= 0) {
			perror("write() failed");
			exit(1);
		}
		donebytes += rc;
	}
}


void ReadDataFromFPGADDR3_thread(int fd32, uint32_t datasize, void *data, int coreid) {


	int rc;
	char *buf;
	buf = (char *) data;
	int donebytes = 0;

	while (donebytes < datasize) {
		rc = read(fd32, buf + donebytes, datasize - donebytes);
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
}


int WriteData2FPGADDR3( fpga_accel_command command, void *data) {


	int status;
	int r;
	boost::thread_group g;
	fpga_accel_command WriteToFPGADDR3_read_back_command;
	pid_t cpid;

	r = sendCommand(&command);
	r = receiveAcknowledge(&WriteToFPGADDR3_read_back_command);

	if (WriteToFPGADDR3_read_back_command.command_id == FPGAMEMORY_OK) {

		unsigned int datasize = (command.variable_data_size / 2);
		void* data_1 = data;
		void* data_2 = data + datasize;

		g.add_thread(new boost::thread(WriteData2FPGADDR3_thread, fdw32_1, datasize, data_1, 1));
		g.add_thread(new boost::thread(WriteData2FPGADDR3_thread, fdw32_2, datasize, data_2, 2));

		g.join_all();
	} else {
		printf("Error in writing data to the FPGA DDR3\n");
	}

	r = receiveAcknowledge(&WriteToFPGADDR3_read_back_command);
	return 0;
}



int ReadDataFromFPGADDR3(fpga_accel_command command, void *data) {




	int status;
	int r;
	boost::thread_group g;
	fpga_accel_command WriteToFPGADDR3_read_back_command;


	fpga_accel_command ReadFromFPGADDR3_read_back_command;


	r = sendCommand(&command);
	r = receiveAcknowledge(&ReadFromFPGADDR3_read_back_command);

//	printf("ReadDataFromFPGADDR3.command_id = %d\n", ReadFromFPGADDR3_read_back_command.command_id);

	if (ReadFromFPGADDR3_read_back_command.command_id == FPGAMEMORY_OK) {

		unsigned int datasize = (command.variable_data_size / 2);
		void* data_1 = data;
		void* data_2 = data + datasize;

		g.add_thread(new boost::thread(ReadDataFromFPGADDR3_thread, fdr32_1, datasize, data_1, 1));
		g.add_thread(new boost::thread(ReadDataFromFPGADDR3_thread, fdr32_2, datasize, data_2, 2));

		g.join_all();


	} else {
		printf("Error in writing data to the FPGA DDR3\n");
	}


	return 0;
}



int fpga_init() {
	fpga_accel_command initFpgaMemoryAllocation_command;

	initFpgaMemoryAllocation_command.command_id = INIT_FPGA;
	sendCommand( &initFpgaMemoryAllocation_command);

	return 0;
}


void* fpga_alloc(uint32_t size, fpga_argument_direction_type direction) {
	uint32_t  *first_arg_address;

	fpga_accel_command fpgaMalloc_command;
	fpga_accel_command fpgaMallocReadBack;


	fpgaMalloc_command.command_id             = MALLOC;   //malloc command
	fpgaMalloc_command.variable_address       = 0;
	fpgaMalloc_command.variable_index         = 0;
	fpgaMalloc_command.variable_type          = POINTER;
	fpgaMalloc_command.variable_direction     = direction;
	fpgaMalloc_command.variable_data          = 0;
	fpgaMalloc_command.variable_data_size     = size; // size in byte

	sendCommand( &fpgaMalloc_command);
	receiveAcknowledge(&fpgaMallocReadBack);

	printf("fpgaMallocReadBack.command_id=%u\n", fpgaMallocReadBack.command_id);
	printf("fpgaMallocReadBack.variable_address=0X%x\n", fpgaMallocReadBack.variable_address);
	printf("fpgaMallocReadBack.variable_data_size=%u\n", fpgaMallocReadBack.variable_data_size);
	first_arg_address = (uint32_t*)(fpgaMallocReadBack.variable_address);


	return (void*)first_arg_address;
}


int fpga_memcpy(void *src, void *dst, uint32_t size, fpga_argument_direction_type direction ) {
	int status=0;
	fpga_accel_command                memcpy_command;
	if (direction==IN) {
//		printf("WRITE_TO_FPGADDR3\n");
		memcpy_command.command_id         = WRITE_TO_FPGADDR3;   //malloc command
		memcpy_command.variable_address   = (intptr_t) dst;
	} else if (direction==OUT){
//		printf("READ_FROM_FPGADDR3\n");
		memcpy_command.command_id         = READ_FROM_FPGADDR3;   //malloc command
		memcpy_command.variable_address   = (intptr_t) src;
	}


	memcpy_command.variable_index         = 0;
	memcpy_command.variable_type          = POINTER;
	memcpy_command.variable_direction     = direction;
	memcpy_command.variable_data          = 0;
	memcpy_command.variable_data_size     = size; // size in byte


	if (direction==IN) {
		status = WriteData2FPGADDR3(memcpy_command,   src);
	} else if (direction==OUT) {
		status = ReadDataFromFPGADDR3(memcpy_command, dst);
	}

	if (!status)
		return 0;
	else
		return -1;
}


int setArg(uint32_t size, fpga_argument_direction_type direction) {
	fpga_accel_command setArg_command;



	setArg_command.command_id             = ACCELERATOR_ARG;   //malloc command
	setArg_command.variable_address       = 0;
	setArg_command.variable_index         = 0;
	setArg_command.variable_type          = POINTER;
	setArg_command.variable_direction     = direction;
	setArg_command.variable_data          = 0;
	setArg_command.variable_data_size     = size; // size in byte
}



int accelerateRun() {

//	printf("Hello accelerateRun\n");

	fpga_accel_command accelerator_start_command;
	fpga_accel_command accelerator_ack_command;

	accelerator_start_command.command_id             = ACCELERATOR_START;   //malloc command


	sendCommand(&accelerator_start_command);
	receiveAcknowledge(&accelerator_ack_command);

	return accelerator_ack_command.command_id;
}



int measure_RTT() {
	int status;

	hardware_start = getTimestamp();

	fpga_accel_command loopback_command;
	fpga_accel_command loopback_acknowledge;
	loopback_command.command_id = LOOP_BACK;

	status = sendCommand(&loopback_command);
	status = receiveAcknowledge(&loopback_acknowledge);

	hardware_end = getTimestamp();
	hardware_execution_time = (hardware_end-hardware_start)/(1000);
	printf("&&&&&&&&& RTT  %.6lf ms elapsed\n", hardware_execution_time);


	return 0;
}
