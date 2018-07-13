/*
 * IPC_Resource.h
 *
 *  Created on: Apr 14, 2018
 *      Author: aimery
 */

#ifndef IPC_RESOURCE_H_
#define IPC_RESOURCE_H_

#include <pthread.h>

#define	IPC_NAME_LEN						(16)
#define	IPC_KEY_WORD_LEN				(32)
#define	MAX_IPC_NUM						(4096)

#define	FTOK_ID									(122)

#define	SHM_HEADER_NAME				"SHM_HEADER_NAME"


typedef enum{
	IPC_SHME = 1,
	IPC_MSGQ,
	IPC_SMPH,
	IPC_SIGN,
	IPC_FILE,
	IPC_SOCK
}IPC_T;

typedef enum{
	RD,
	WR,
	XT,
	RW,
	RX,
	WX,
	RWX,
	DEN  //deny
}PERMS_T;


/*about state type*/
typedef enum{
	HEALTH,
	//READY,		//Resource allocated and ready for next jobs
	UNBORN,		//Resource is not generated for incorrect apply
	SICK, 			//Unable to be linked in or out ruler
/* above need to be released */
	STANDBY,	//Haven't allocated resource, and waitting for resource
	ZOMBIA,		//Unable to release resource by OS
	DIE	 			//Released and detached from ruler
}STATE;

typedef struct shmHead{
	shmHead * 			prv;
	shmHead * 			next;
	char   					shmName[IPC_NAME_LEN];

	int			 			shmId;
	key_t  					ipcKey;
	size_t 					shmSize;
	STATE	 				shmState;

	unsigned 			totalCnt;
	pthread_rwlock_t shmRWLock;
	char 					bookmark[0];

}ShmHead_t;


#ifdef __cplusplus__
extern  "C" {
#endif




#ifdef __cplusplus__
}
#endif



#endif /* IPC_RESOURCE_H_ */
