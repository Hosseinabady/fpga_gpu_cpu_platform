/**
 * @file   testebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the ebbchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/ebbchar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include "time.h"
#include <sys/time.h>

double start;
double end;
double execution_time;
double getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1e6;
}

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(){
   int ret, fd;
   char stringToSend[BUFFER_LENGTH] = "This is a test.";
   fd = open("/dev/ebbchar", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }


   int n = 10;

   for (int i = 0; i < n; i++) {
	   start = getTimestamp();
	   ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
	   if (ret < 0){
		  perror("Failed to write the message to the device.");
		  return errno;
	   }
	   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
	   if (ret < 0){
		  perror("Failed to read the message from the device.");
		  return errno;
	   }
	   end = getTimestamp();
	   execution_time = (end-start)/(1000);
	   printf("write-read execution time  %.6lf ms elapsed\n", execution_time);
   }

   return 0;
}
