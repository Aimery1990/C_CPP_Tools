/*
 * File_Operation.c
 *
 *  Created on: Mar 11, 2018
 *      Author: aimery
 */



#include<sys/stat.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/time.h>
#include<pthread.h>
#include<sys/msg.h>
#include<errno.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<math.h>

#include"common.h"
#include"File_Operation.h"


#define FILE_FLAGS_NEW			O_WRONLY | O_CREAT | O_TRUNC
#define FILE_FLAGS_FLAG			O_RDONLY | O_CREAT
#define FILE_FLAGS_APP			O_WRONLY | O_CREAT | O_APPEND
#define FILE_FLAGS_READ			O_RDONLY
#define FILE_MODE			 		S_IRWXU | S_IRGRP | S_IROTH




int is_accessable_file(const char * filepath, ACCESS_T acs )
{
	if(!filepath){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	int mode = 0;
	switch (acs)
	{
	case IS_EXISTED:
		mode = F_OK;
		break;
	case IS_READ:
		mode = R_OK;
		break;
	case IS_WRITTEN:
		mode = W_OK;
		break;
	case IS_EXECUTED:
		mode = X_OK;
		break;
	default:
		PRINTLOG(ERROR, "Call error, Invalid access type query");
		return PARAM_ERROR;
	}

	if(access(filepath, mode) == -1){
		PRINTLOG(NOTE, "%s don't have the access of mode %d", filepath, mode);
		return AVAILABLE_ERROR;
	}
	return SUCCESS;
}



int new_empty_file(const char * filepath)
{
	if(!filepath){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	int fd = open(filepath, FILE_FLAGS_NEW, FILE_MODE);
	if(fd<0){
		PRINTLOG(ERROR, "open error");
		return SYS_ERROR;
	}
	close(fd);
	return SUCCESS;
}


/*
int new_flag_file(const char * filepath)
{
	if(!filepath){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	int fd = open(filepath, FILE_FLAGS_FLAG);
	if(fd<0){
		PRINTLOG(ERROR, "open error");
		return SYS_ERROR;
	}

	return SUCCESS;
}
*/

int app_open_file(const char * filepath)
{
	if(!filepath){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	int fd = open(filepath, FILE_FLAGS_APP);
	if(fd<0){
		PRINTLOG(ERROR, "open error");
		return SYS_ERROR;
	}
	return fd;
}



int read_open_file(const char * filepath)
{
	if(!filepath){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	int fd = open(filepath, FILE_FLAGS_READ);
	if(fd<0){
		PRINTLOG(ERROR, "open error");
		return SYS_ERROR;
	}
	return fd;
}



int app_write_file(int fd, char * buffer, unsigned len)
{
	if( fd<0 || !buffer || len<=0 ){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	unsigned to = 0;
	char * ptr = buffer;
	int written = 0;

	while(to<len)
	{
		written = write( fd, ptr, len-to);
		if(written <= 0){
			if(errno == EINTR)
				continue;
			close(fd);
			PRINTLOG(ERROR, "write error");
			return SYS_ERROR;
		}
		to += written;
		ptr += to;
	}
	return SUCCESS;
}


int only_read_file(int fd, char * buffer, unsigned len)
{
	if( fd<0 || !buffer || len<=0 ){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	unsigned to = 0;
	char * ptr = buffer;
	int rd = 0;

	while(to<len)
	{
		rd = read(fd, ptr, len-to);
		if(rd<0){
			if(errno == EINTR)
				continue;
			close(fd);
			PRINTLOG(ERROR, "read error");
			return SYS_ERROR;
		}
		to += rd;
		ptr += rd;
		if(rd == 0){
			break;
		}
	}

	return to;
}

#if 0
int main()
{
	int ret = 0;
/*	int fd = 0;
	ret = new_empty_file("hello_test.log");
	fd = app_open_file("hello_test.log");
	ret = app_write_file(fd, "hello_test.log\n", strlen("hello_test.log\n"));
	PRINTLOG(DEBUG, "ret is %d", ret);
	ret = app_write_file(fd, "hello_test.log\n", strlen("hello_test.log\n"));
	PRINTLOG(DEBUG, "ret is %d", ret);
	close(fd);
	char buffer[1024] = {0};
	fd = read_open_file("hello_test.log");
	ret = only_read_file(fd, buffer, 1024);
	PRINTLOG(DEBUG, "ret is %d", ret);
	PRINTLOG(DEBUG, "buffer is %s", buffer);
	close(fd);
*/

	ret = is_accessable_file("hello_test.log", IS_EXISTED);
	if(ret == SUCCESS){
		PRINTLOG(NOTE, "hello_test.log is existed");
	}
	else{
		PRINTLOG(NOTE, "hello_test.log isn't existed");
	}


	ret = is_accessable_file("hello_test.log", IS_WRITTEN);
	if(ret == SUCCESS){
		PRINTLOG(NOTE, "hello_test.log is writable");
	}
	else{
		PRINTLOG(NOTE, "hello_test.log isn't writable");
	}

	ret = is_accessable_file("hello_test.log", IS_READ);
	if(ret == SUCCESS){
		PRINTLOG(NOTE, "hello_test.log is readable");
	}
	else{
		PRINTLOG(NOTE, "hello_test.log isn't readable");
	}


	return 0;
}
#endif


