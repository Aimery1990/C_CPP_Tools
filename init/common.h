/*
 * common.h
 *
 *  Created on: Mar 1, 2018
 *      Author: aimery
 */

#ifndef COMMON_H_
#define COMMON_H_


#define ERROR	"ERROR"
#define WARN	"WARN"
#define NOTE		"NOTE"
#define DEBUG	"DEBUG"


#define PRINTLOG(LEVEL, desc, params...)		do{	\
																			fprintf( stderr, "%s\t%s\t%s\t%d ",LEVEL,__FILE__,__FUNCTION__,__LINE__);	\
																			fprintf( stderr, desc, ##params );	\
																			fprintf( stderr, "\n");	\
																			}while(0);

#define FREE(ptr)												if(ptr) {free(ptr);  ptr = NULL;}

#define	K			(1024)
#define	M			(1024*K)
#define	G			(1024*M)

#define	US		(1)
#define	MS		(1000)
#define	SEC		(1000000)

#define	TRUE	(0x0001)
#define	FALSE	(0x0000)

typedef	int		bool_t;



typedef enum{
	AVAILABLE_ERROR = -5,
	FUNC_ERROR = -4,
	PARAM_ERROR = -3,
	SYS_ERROR = -2,
	LOGIC_ERROR = -1,
	SUCCESS = 0
}RET_VAL;


typedef enum{
	SHORT_MSG,		/*less than or equal to 1 K*/
	LONG_MSG,			/*less than 10 K*/
	SHORT_FILE,		/*less than 10 M*/
	LONG_FILE,			/*less than 4 G*/
	STREAM,
	TASK_NONE
}TASK_T;

typedef enum{
	EMERGENCY=0,
	URGENT=4,
	REALTIME=8,
	NORMAL_DELIVERY=16,
	DELAY_DELIVERY=32
}PRIOR_T;


typedef struct taskinfo{
	TASK_T task_type;
	PRIOR_T task_priority;
	unsigned task_sender;
	unsigned task_receiver;
	unsigned long task_size;
	unsigned long task_id;
}TASK_INFO_T;


typedef struct threadData{
	TASK_INFO_T task_info;
	unsigned long data_progress;
	unsigned long data_size;
	char * data;
}THRD_DATA_T;

#ifdef __cplusplus__
extern  "C" {
#endif




#ifdef __cplusplus__
}
#endif




#endif /* COMMON_H_ */
