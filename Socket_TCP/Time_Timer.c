/*
 * Time_Timer.c
 *
 *  Created on: Mar 12, 2018
 *      Author: aimery
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>

#include"common.h"


time_t get_unix_time_stamp()
{
	time_t seconds = time(NULL);
	PRINTLOG(DEBUG, "time stamp is %ld", seconds);
	return seconds;
}

unsigned long get_unix_ms_time_stamp()
{
	struct timeval time = {0};
	unsigned long stamp = 0UL;
	if(0 == gettimeofday(&time, NULL)){
		stamp = time.tv_sec << 24;
		stamp += time.tv_usec;
		PRINTLOG(DEBUG, "ms time stamp is %ld tv_sec is %ld tv_usec is %d", stamp, time.tv_sec, time.tv_usec);
		return stamp;
	}
	else{
		PRINTLOG(ERROR, "gettimeofday() error");
	}
	return stamp;
}

int translate_unix_ms_time_stamp( unsigned long timeStamp, struct timeval * time )
{
	if(time == NULL){
		PRINTLOG(ERROR, "Call error.");
		return PARAM_ERROR;
	}

	time->tv_sec = timeStamp>>24;
	time->tv_usec = timeStamp - (time->tv_sec<<24);
	PRINTLOG(DEBUG, "SEC is %ld, USEC is %d", time->tv_sec, time->tv_usec);
	return SUCCESS;
}

clock_t get_program_useconds()
{
	clock_t cpu_clock = clock();
	if(cpu_clock != -1){
		PRINTLOG(DEBUG, "cpu useconds since program start: %ld", cpu_clock);
	}
	else{
		PRINTLOG(ERROR, "cpu time is unavailable");
	}
	return cpu_clock;
}

bool_t us_timer(long start, long useconds)
{
	clock_t period = get_program_useconds() - start;
	if( period >= useconds)
		return TRUE;
	else
		return FALSE;
}

/*
#include<math.h>
#define MAX_SIZE												((unsigned long)(256*256*256))*((unsigned long)(256*256*256))*(unsigned long)(256)
#define COUNT_BUF_SIZE									(7)

unsigned long count_total( const unsigned char * buffer, const unsigned buffer_size, unsigned scale )
{
	if(scale>256||!buffer||buffer_size>COUNT_BUF_SIZE){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	int i = 0;
	unsigned long total = 0;
	unsigned long power = 1;
	for( ; i < buffer_size; i ++ )
	{
		power = pow( scale, buffer_size-1-i );
		total += buffer[i] * power;
	}
	PRINTLOG(DEBUG, "total is %lu", total);
	return total;
}


int count_buffer(unsigned long total, unsigned char * buffer, unsigned buffer_size, unsigned scale )
{
	PRINTLOG(DEBUG, "MAX_SIZE is %lu", MAX_SIZE );
	if(scale>256 || total>MAX_SIZE || !buffer || buffer_size>COUNT_BUF_SIZE){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}
	unsigned long power = 0;
	unsigned long tmp = 0;
	int i = 0;

	for( ; i < buffer_size; i++)
	{
		power = pow( scale, buffer_size-1-i);
		tmp = total/power;
		if(tmp)
		{
			buffer[i] = tmp;
			total = total - power * tmp;
		}
	}

	for( i = 0; i< buffer_size; i++)
	{
		PRINTLOG(DEBUG, "%d ", buffer[i]);
	}
	return SUCCESS;
}
*/


# if 0
int main()
{
	get_unix_time_stamp();
	clock_t start = get_program_useconds();
	while(!us_timer(start, 200))
	{
		//usleep(10);
	}
	get_program_useconds();

	unsigned char buffer[7] = {0};
	unsigned long timeStamp = get_unix_time_stamp();
	//timeStamp = 4102416000UL;

	struct timeval time = {0};

	start = get_program_useconds();
	while(!us_timer(start, 500))
	{
		timeStamp = get_unix_ms_time_stamp();

		translate_unix_ms_time_stamp(timeStamp, &time);

		//time.tv_sec = timeStamp>>16;
		//time.tv_usec = timeStamp - (time.tv_sec<<16);
		//PRINTLOG(DEBUG, "SEC is %ld, USEC is %d", time.tv_sec, time.tv_usec);

		count_buffer(timeStamp, buffer, 7, 256);
		timeStamp = count_total(buffer, 7, 256);
	}




	return SUCCESS;


}
#endif
