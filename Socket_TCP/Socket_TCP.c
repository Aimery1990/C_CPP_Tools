/*
 * Socket_TCP.c
 *
 *  Created on: Feb 25, 2018
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
#include"config.h"

#include"Time_Timer.h"
//#include"File_Operation.h"


#include"Socket_TCP.h"

static unsigned thread_count = 0;
static unsigned ALL_THREAD_QUIT = 0;
static unsigned char  dockers_burden[MAX_DOCKERS] = {0}; /*dockers' thread load*/
static THRD_DATA_T thread_data[MAX_DOCKERS] = {0};
static int existed_dockers = 0;

/*
 * task type: unsigned char
 * total size: unsigned char array, length is 6
 * priority:	unsigned char
 * Sender:	unsigned char array, length is 2
 * Receiver:	unsigned char array, length is 2
 * Client task id:
 * Reserved: unsigned char array, length is 4
 *
 * */
#define TASK_TYPE_LEN										(1)
#define TASK_SIZE_LEN										(6)
#define TASK_PRIO_LEN										(1)
#define TASK_SNDR_LEN									(2)
#define TASK_RCVR_LEN									(2)
#define TASK_ID_LEN											(7)	/**/
#define TASK_RSVR_LEN									(4)
#define TASK_INFO_LEN										(23)
#define MAX_SIZE												((unsigned long)(256*256*256))*((unsigned long)(256*256*256))*(unsigned long)(256)
#define COUNT_BUF_SIZE									(7)
#define NUMERIC_SCALE									(256)


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


int copyman( const int size, const char * src, char * dst, int * len )
{
   if( size<=0 || src==NULL || dst == NULL  ){
      PRINTLOG(ERROR, "Call error");
      return PARAM_ERROR;
   }
   int increase = 0;
   if(size<BUF_SIZE){
      memcpy( dst, src, size );
      increase+=size;
   }
   else{
      int cptms = size/BUF_SIZE;
      int remainder = size%BUF_SIZE;
      int i = 0;
      for( ; i<cptms; i++)
      {
         memcpy( dst+increase, src+increase, BUF_SIZE);
         increase += BUF_SIZE;
      }
      memcpy( dst+increase, src+increase, remainder );
      increase += remainder;
   }
   if(len)
      *len = increase;
   return SUCCESS;
}




int parse_task_info_type( unsigned char * buffer, TASK_INFO_T * task_info )
{
	if( !buffer || !task_info){
		PRINTLOG(ERROR, "Call error");
		return PARAM_ERROR;
	}

	unsigned char * ptr = buffer;
	task_info->task_type = *ptr;
	ptr+=TASK_TYPE_LEN;
	task_info->task_size = count_total( ptr, TASK_SIZE_LEN, NUMERIC_SCALE);
	ptr+=TASK_SIZE_LEN;
	task_info->task_priority = *ptr;
	ptr+=TASK_PRIO_LEN;
	task_info->task_sender = (unsigned)count_total(ptr, TASK_SNDR_LEN, NUMERIC_SCALE);
	ptr+=TASK_SNDR_LEN;
	task_info->task_receiver = (unsigned)count_total(ptr, TASK_RCVR_LEN, NUMERIC_SCALE);
	ptr+=TASK_RCVR_LEN;
	task_info->task_id = count_total(ptr, TASK_ID_LEN, NUMERIC_SCALE);

	return SUCCESS;
}


int check_receiver( unsigned receiver )
{

	return SUCCESS;
}

int get_available_dockers_thread( PRIOR_T prior, unsigned long timeout )
{
	int i = 0;
	int ret = 0;
	clock_t start = get_program_useconds();

	while(!us_timer(start, timeout))
	{
		switch (prior)
		{
		case EMERGENCY:
			for( i = EMERGENCY; i< existed_dockers; i++)
			{
				/*find the first free dockers thread*/
				if(dockers_burden[i] == 1){
					dockers_burden[i]++;
					return i;
				}
			}
			ret = Create_Dockers_Threads_Pool(1);
			if(ret != SUCCESS){
				return ret;
			}

			break;

		case URGENT:
			for( i = URGENT; i< existed_dockers; i++)
			{
				/*find the first free dockers thread*/
				if(dockers_burden[i] == 1){
					dockers_burden[i]++;
					return i;
				}
			}
			ret = Create_Dockers_Threads_Pool(1);
			if(ret != SUCCESS){
				return ret;
			}

			break;

		case REALTIME:
			for( i = REALTIME; i< existed_dockers; i++)
			{
				/*find the first free dockers thread*/
				if(dockers_burden[i] == 1){
					dockers_burden[i]++;
					return i;
				}
			}
			ret = Create_Dockers_Threads_Pool(1);
			if(ret != SUCCESS){
				return ret;
			}

			break;

		case NORMAL_DELIVERY:
			for( i = NORMAL_DELIVERY; i< existed_dockers; i++)
			{
				/*find the first free dockers thread*/
				if(dockers_burden[i] == 1){
					dockers_burden[i]++;
					return i;
				}
			}
			ret = Create_Dockers_Threads_Pool(1);
			if(ret != SUCCESS){
				return ret;
			}

			break;

		case DELAY_DELIVERY:
			for( i = DELAY_DELIVERY; i< existed_dockers; i++)
			{
				/*find the first free dockers thread*/
				if(dockers_burden[i] == 1){
					dockers_burden[i]++;
					return i;
				}
			}
			break;

		default:
			PRINTLOG(ERROR, "Priority type error!");
			return PARAM_ERROR;
		}
		continue;
	}
	return AVAILABLE_ERROR;
}

int add_into_dockers_thread( TASK_INFO_T * task_info, char  * data, unsigned long data_size )
{
	int docker_id = get_available_dockers_thread(task_info->task_priority, MS);
	if(docker_id<0){
		PRINTLOG(ERROR, "Unable to get available dockers thread with ret %d", docker_id);
		return docker_id;
	}

	thread_data[docker_id].task_info = * task_info;
	thread_data[docker_id].data_size = data_size;
	thread_data[docker_id].data = data;

	return SUCCESS;
}

int proccess_SHORT_MSG( int nfd, TASK_INFO_T * task_info )
{
	int ret = 0;
	int data_sz = 0;
	char * buffer = (char *)malloc(BUF_SIZE);			//TODO FLAG to be free after being processed successfully
	if(!buffer){
		PRINTLOG(ERROR, "Fail to malloc");
		return SYS_ERROR;
	}
	memset( buffer, 0, BUF_SIZE );

	ret = TCP_Send(nfd, READY_FOR_RECV, sizeof(READY_FOR_RECV));
	if(ret != sizeof(READY_FOR_RECV)){
		FREE(buffer);
		PRINTLOG(ERROR, "Fail to send ready for recv info");
		return FUNC_ERROR;
	}

	data_sz = TCP_Recv( nfd, buffer, task_info->task_size);
	if(data_sz == task_info->task_size){
		ret = add_into_dockers_thread( task_info, buffer, data_sz );
		if(ret != SUCCESS){
			FREE(buffer);
			PRINTLOG(ERROR, "Fail to add into dockers thread");
			return ret;
		}
	}
	else{
		FREE(buffer);
		PRINTLOG(ERROR, "Fail to receive correct data len");
		return data_sz;
	}

	return SUCCESS;
}

int prepare_for_task( int nfd, TASK_INFO_T * task_info )
{
	switch (task_info->task_type)
	{
	case SHORT_MSG:
		if( SUCCESS != proccess_SHORT_MSG(nfd, task_info) ){
			PRINTLOG(DEBUG, "Fail to proccess short msg");
			return AVAILABLE_ERROR;
		}
		break;
	case LONG_MSG:

		break;
	case SHORT_FILE:

		break;
	case LONG_FILE:

		break;
	case STREAM:

		break;
	default:
		break;
	}

	return SUCCESS;
}


int process_task_info_type( int nfd, TASK_INFO_T * task_info)
{
	int ret = 0;
	char buffer[BUF_SIZE] = {0};
	//TASK_INFO_T task_type_info = {0};
	ret = TCP_Recv( nfd, buffer, TASK_INFO_LEN);
	if(ret == TASK_INFO_LEN){
		ret = parse_task_info_type( buffer, task_info);
		if(ret == SUCCESS){
			PRINTLOG(NOTE, "parse_task_info_type() success");
			PRINTLOG(NOTE, "task_type is %d, task_size is %lu, task_priority is %d, task_sender is %d, task_receiver is %d, task_id is %lu",
					task_info->task_type, task_info->task_size, task_info->task_priority, task_info->task_sender, task_info->task_receiver, task_info->task_id);
		}
		else{
			PRINTLOG(ERROR, "Fail to parse_task_info_type() thread will quit ret: %d",  ret);
			return ret;
		}
	}
	else{
		PRINTLOG(ERROR, "Fail to recv task info type for ret: %d thread will quit", ret);
		return ret;
	}

	ret = prepare_for_task( nfd, task_info );
	if(ret != SUCCESS){
		PRINTLOG(DEBUG, "Fail to prepare for task with ret: %d", ret);
		return ret;
	}

	return SUCCESS;
}


int TCP_Recv( int fd, char * buf, int len )
{
   if( buf == NULL ){
	  PRINTLOG(ERROR, "Call error.");
      return PARAM_ERROR;
   }

   int ret = 0;
   int count = 0;
   int try = 3;
   struct timeval period;
   fd_set readFd;

   while(try--)
   {
      FD_ZERO(&readFd);
      FD_SET( fd, &readFd );
      period.tv_sec = 0;
      period.tv_usec = 50*1000;

      ret = select( fd+1, &readFd, NULL, NULL, &period );
      if( ret==0 ){
         continue;
      }
      if( ret<0 ){
         PRINTLOG(ERROR, "select() failed");
         return SYS_ERROR;
      }

      if(FD_ISSET( fd, &readFd)){
         ret = recv( fd, buf+count, len-count, 0 );
         if( ret<=0 ){
            if((errno == EAGAIN)||(errno == EWOULDBLOCK)||(errno == EINTR)){
               continue;
            }
            PRINTLOG(ERROR, "recv() failed");
            return SYS_ERROR;
         }
         count += ret;
         if( count == len ){
               break;
         }
      }
   }
   if(!try){
	   PRINTLOG(ERROR, "Unable to recv for timeout");
	   return SOCKET_ERROR_TIMEOUT;
   }

   return count;
}


int TCP_Send( int fd, const char * buf, int len)//risky
{
   if( !buf ){
	  PRINTLOG(ERROR, "Call error.");
      return PARAM_ERROR;
   }

   int to = 0;
   int ret = 0;
   while( to<len )
   {
      ret = send( fd, buf+to, len-to, 0 );
      if( ret==-1 ){
         if( errno == EINTR || errno == EWOULDBLOCK ){
            to = 0;
            continue;
         }
         PRINTLOG(ERROR, "send() failed");
         return SYS_ERROR;
      }
      to += ret;
   }
   return to;
}


void dockers_proccess_callback( void * docker_idx )
{
	int docker_num = *(int *) docker_idx;
	free(docker_idx);

	dockers_burden[docker_num] = 1;

	while(1)
	{

	}



}

void client_proccess_callback( void * nP )
{
	int nfd = *(int *)nP;
	free(nP);
	thread_count++;
	int ret = 0;
	char buffer[BUF_SIZE] = {0};
	unsigned task_fail = 0;
	fd_set fdset;
	struct timeval period;
	//TASK_T task_type = TASK_NONE;
	TASK_INFO_T task_type_info = {0};

	PRINTLOG(NOTE, "nfd is %d, thread_count is %u", nfd, thread_count);

	while(!ALL_THREAD_QUIT)
	{
		period.tv_sec = 10;
		period.tv_usec = 0;

		FD_ZERO(&fdset);
		FD_SET( nfd, &fdset );
		ret = select( nfd+1, &fdset, NULL, NULL, &period );
		if( ret<=0 || !(FD_ISSET( nfd, &fdset)) ){
			if(ret == 0){
				task_fail = 1;
				PRINTLOG(DEBUG, "Task failed, select() timeout");
				break;
				//continue;
			}
			task_fail = 1;
			PRINTLOG(ERROR, "Task failed, select() error");
			break;
		}











	}

	FD_ZERO(&fdset);
	close(nfd);

	thread_count--;
}



int Create_Dockers_Threads_Pool( unsigned num )
{
	if( num <=0 || num > MAX_DOCKERS){
		PRINTLOG(ERROR, "Call error, invalid initialized thread number which is %d", num);
		return PARAM_ERROR;
	}

	int ret = 0;
	int * docker_idx = NULL;
	pthread_t tid = 0;
	static int i = 0;
	existed_dockers += num;
	if(existed_dockers > MAX_DOCKERS){
		PRINTLOG(ERROR, "Unable to create such amount of dockers' thread");
		return LOGIC_ERROR;
	}

	for( ; i < existed_dockers; i++)
	{
		docker_idx  = (int *)malloc( sizeof(int) );
        if( docker_idx == NULL ){
           PRINTLOG( ERROR, "API error, malloc() fails");
           return SYS_ERROR;
        }
        *docker_idx = i;
        tid = 0;
        ret = pthread_create(&tid, NULL, dockers_proccess_callback, (void*)docker_idx );
        if(ret!=0){
           PRINTLOG( ERROR, "Fail to create new thread\n");
           return SYS_ERROR;
        }
        if( pthread_detach(tid) ){
           PRINTLOG( ERROR, "Fail to detach a thread\n");
           return SYS_ERROR;
        }
	}
	return SUCCESS;
}

int Create_TCP_Server_Socket_FD( const char * ip, const unsigned port )
{
   if( strcmp( ip, TCP_IP_ADDR ) || port != TCP_PORT ){
	  PRINTLOG( ERROR, "Call error, invalid IP address or port");
	  return PARAM_ERROR;
   }

	int socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(socket_fd < 0){
		PRINTLOG( ERROR, "API error, socket() fails");
		close(socket_fd);
		return SYS_ERROR;
	}

	struct sockaddr_in server = {0};
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if( 1!= inet_pton( AF_INET, ip, &(server.sin_addr.s_addr) )){
		PRINTLOG(ERROR, "API error, inet_pton() fails");
		close(socket_fd);
		return SYS_ERROR;
	}

	unsigned value = 1;
	if( 0 != setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))){
		PRINTLOG(ERROR, "API error, setsockopt() fails");
		close(socket_fd);
		return SYS_ERROR;
	}

	if( 0 != bind( socket_fd, (struct sockaddr*)&server, sizeof(server) )){
		PRINTLOG(ERROR, "API error, bind() fails");
		close(socket_fd);
		return SYS_ERROR;
	}

	if( 0 != listen( socket_fd, 20 )){
		PRINTLOG(ERROR, "API error, listen() fails");
		close(socket_fd);
		return SYS_ERROR;
	}

	return socket_fd;
}

int  TCP_Server_Accepter( int socket_fd, void * callback )
{
	if( socket_fd<=2 ){
	  PRINTLOG(ERROR, "Call error, invalid socket descriptor\n");
	  return PARAM_ERROR;
	}

   struct timeval period, tperiod;
   period.tv_sec = 3;
   period.tv_usec = 0;
   tperiod = period;

   int maxFd = socket_fd;
   fd_set set, tset;
   FD_ZERO( &set );
   FD_ZERO( &tset );
   FD_SET( socket_fd, &set );

   struct sockaddr_in client = {0};
   char weird = 100;
   int ret = 0;
   pthread_t tid = 0;
   int nfd = -1;
   //unsigned tidCount = 0;

   while(  weird>=0  || !ALL_THREAD_QUIT)
   {
      tset = set;
      ret = select( maxFd+1, &tset, NULL, NULL, &tperiod );
      if( 0 > ret ){
         PRINTLOG(ERROR, "API error, Select() fails");
         if( errno == EINTR){
            PRINTLOG( WARN, "EINTR caught");
         }
         weird--;
         tperiod = period;
         continue;
      }
      if( ret==0 ){
         PRINTLOG(DEBUG, "select timeout");
         tperiod = period;
         continue;
      }

      if( FD_ISSET( socket_fd, &tset ) ){

         int sockLen = sizeof(client);
         memset( &client, 0, sockLen );

         nfd = accept( socket_fd, (struct sockaddr *)&client, &sockLen );
         if( nfd<0 ){
            PRINTLOG( ERROR, "API error, accept() fails");
            weird--;
            continue;
         }

         unsigned long ul = 1;
	     ioctl(nfd, FIONBIO, &ul);

         PRINTLOG(NOTE, "New client Accepted");

         int * nP = (int *)malloc( sizeof(int) );
         if( nP == NULL ){
            PRINTLOG( ERROR, "API error, malloc() fails");
            break;
         }
         *nP = nfd;

         tid = 0;
         ret = pthread_create(&tid, NULL, callback, (void*)nP );
         if(ret!=0){
            PRINTLOG( ERROR, "Fail to create new thread\n");
            break;
         }
         if( pthread_detach(tid) ){
            PRINTLOG( ERROR, "Fail to detach a thread\n");
            break;
         }

      }
      weird = 100;
   }

   int tolerance = 300;
   ALL_THREAD_QUIT = 1;
   while( thread_count && --tolerance>0 ){
      PRINTLOG(ERROR, "There are %d sub_threads left in local TCP server", thread_count );
      usleep(10000);
   }

   /*......to be continued......*/
   FD_ZERO( &set );
   FD_ZERO( &tset );
   close( socket_fd );
   PRINTLOG(ERROR, "Local TCP quit");

   return SUCCESS;
}

int Launch_TCP_Server( const char * ip, const unsigned port )
{
	   if( strcmp( ip, TCP_IP_ADDR ) || port != TCP_PORT ){
		  PRINTLOG( ERROR, "Call error, invalid IP address or port");
		  return PARAM_ERROR;
	   }

	   	int ret = 0;
		int socket_fd = Create_TCP_Server_Socket_FD( ip, port );
		if(socket_fd < 0){
			PRINTLOG( ERROR, "FUNC error, Create_TCP_Server_Socket_FD() fails");
			close(socket_fd);
			return FUNC_ERROR;
		}

		ret = Create_Dockers_Threads_Pool(INITIAL_DOCKERS);
		if(ret != SUCCESS ){
			PRINTLOG(ERROR, "Fail to create dockers thread for %d", ret);
			close(socket_fd);
			return ret;
		}

		TCP_Server_Accepter(socket_fd, client_proccess_callback);

		return socket_fd;
}

int main()
{
	return Launch_TCP_Server(TCP_IP_ADDR, TCP_PORT);
}















