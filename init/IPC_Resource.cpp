/*
 * IPC_Resource.cpp
 *
 *  Created on: Apr 14, 2018
 *      Author: aimery
 */



#include<iostream>

//#include<vector>
#include<list>
#include<algorithm>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<errno.h>

#include"common.h"
//#include"Basic_Algorithm.h"
#include"tinyUtils.h"
#include"File_Operation.h"
#include"IPC_Resource.h"

using namespace std;

#define 	MAX_SHM_NUM 			(1024)
#define	SHM_SIZE_DEF			(4096)


class IPC_Fund{

private:

protected:
public:
	char ipcName[IPC_NAME_LEN];
	char ipcKeyWord[IPC_KEY_WORD_LEN];
	unsigned ipcNameLen;
	IPC_T ipcType;
	size_t ipcSize;
	key_t  ipcKey;

public:
	IPC_Fund( const char * name, unsigned nameLen, IPC_T type, size_t size): ipcNameLen(nameLen), ipcType(type), ipcSize(size),ipcKey(0)
	{
		if(nameLen>IPC_NAME_LEN){
			ipcNameLen = IPC_NAME_LEN;
			PRINTLOG(WARN, "the given ipc name is too long");
		}

		int ret = 0;
		int pos = 0;
		memset(ipcKeyWord, 0 ,sizeof(ipcKeyWord));
		memset(ipcName, 0 ,sizeof(ipcName));
		memcpy( ipcName, name, ipcNameLen );

		sprintf( ipcKeyWord, "%d", ipcType);
		pos = findZeroPos( (const char *)ipcKeyWord, strlen(ipcKeyWord)+1 );

		snprintf( ipcKeyWord + pos, ipcNameLen,  "%s", ipcName);
		pos = findZeroPos( (const char *)ipcKeyWord, strlen(ipcKeyWord)+1 );

		sprintf( ipcKeyWord+pos, "%lu", ipcSize );

		PRINTLOG(DEBUG, "sizeof(IPC_T) is %lu,  ipcName is %s, ipcSize is %lu", sizeof(IPC_T), ipcName, ipcSize);
		PRINTLOG(DEBUG, "keyword is %s", ipcKeyWord );

		ret = is_accessable_file( ipcKeyWord, IS_EXISTED);
		if(ret != SUCCESS){
			ret = new_empty_file(ipcKeyWord);
			if(ret!=SUCCESS){
				PRINTLOG(ERROR, "new_empty_file error");
				return;
			}
		}

		ipcKey = ftok( ipcKeyWord, FTOK_ID);

		PRINTLOG(DEBUG, "the.ipcKey is %d", ipcKey);

	}

	const IPC_Fund & Get_IPC_Fund()
	{
		return  *this;
	}


};


#if 1

class Shmkits{

private:
	char 					shmName[IPC_NAME_LEN];
	int			 			shmId;
	key_t  					ipcKey;
	size_t 					shmSize;
	char *  		 		shmAddr;
	char * 					usrAddr;
	STATE	 				shmState;
	struct shmid_ds	statBuf;
	bool_t 					shmkitError;
	bool_t					shmCnted;

	static char					shmKLHeaderName[IPC_NAME_LEN];
	static int						shmKLHeaderId;
	static ShmHead_t *		shmKLHeaderPtr;
	static bool_t				shmKLHeaderExisted;
	//static list<Shmkits> 	shmKL;


public:
	/*bingo*/
	Shmkits():ipcKey(0),shmId(-1),shmSize(0),shmAddr(NULL), usrAddr(NULL),/*pos(0),shmIdx(0),*/shmState(STANDBY),shmkitError(FALSE)
	{
		int ret = 0;
		memset(shmName, 0, sizeof(shmName));
		memset(&statBuf, 0, sizeof(statBuf));

		if(sizeof(shmKLHeaderName) > strlen(SHM_HEADER_NAME))
			memcpy(shmKLHeaderName, SHM_HEADER_NAME, strlen(SHM_HEADER_NAME));
		else{
			memcpy(shmKLHeaderName, SHM_HEADER_NAME, sizeof(shmKLHeaderName));
		}

		if(!shmKLHeaderExisted){
			if( SUCCESS != createShmKLHeader() ){
				this->shmkitError = TRUE;
				PRINTLOG(ERROR, "Unable to createShmKLHeader()");
				return;
			}
		}
	}

	/**/
	Shmkits( const char * shmName, size_t shmSize = SHM_SIZE_DEF/*, unsigned pos = 0*/)
	{
		bool abnormal = false;
		int	 ret = 0;
		char temp[IPC_NAME_LEN+1] = {0};

		if(sizeof(shmKLHeaderName) > strlen(SHM_HEADER_NAME))
			memcpy(shmKLHeaderName, SHM_HEADER_NAME, strlen(SHM_HEADER_NAME));
		else{
			memcpy(shmKLHeaderName, SHM_HEADER_NAME, sizeof(shmKLHeaderName));
		}

		if(!shmKLHeaderExisted){
			if( SUCCESS != createShmKLHeader() ){
				this->shmkitError = TRUE;
				PRINTLOG(ERROR, "Unable to createShmKLHeader()");
				return;
			}
		}

		do{
			if(shmName!=NULL){

				if(sizeof(this->shmName) > strlen(shmName))
					memcpy(this->shmName, shmName, strlen(shmName));
				else{
					memcpy(this->shmName, shmName, sizeof(this->shmName));
				}
			}
			else{
				memset(this->shmName, 0, sizeof(this->shmName));
				shmState = UNBORN;
				PRINTLOG(ERROR, "Call error, none shmName");
				break;
			}

			if( shmSize<=32*M && shmSize>0 )
				this->shmSize = shmSize;
			else{
				this->shmSize = 0;
				shmState = UNBORN;
				PRINTLOG(ERROR, "Call error, shmSize error");
				break;
			}

			memset(&(this->statBuf), 0, sizeof(statBuf));
			this->ipcKey = 0;
			this->shmId = -1;
			this->shmAddr = NULL;
			this->usrAddr = NULL;
			this->shmCnted = FALSE;
			this->shmkitError = FALSE;
			this->shmState = HEALTH;

			ret = isInShmKL(*this);
			if(TRUE == ret){///////////////////

				/*PERMISSION CONSIDERATION*/
				/*
				if(pos>this->shmSize && pos<0){
					PRINTLOG(WARN, "The given position is out of shm, pos is set into 0");
					this->pos = 0;
				}*/
				if(this->shmState==HEALTH){
					ret = addIntoShmKL(*this); ///////////////////////////////////?????
					if(ret != SUCCESS){
						this->shmState = UNBORN;
						PRINTLOG(ERROR, "Unable to addIntoShmKL()");
						break;
					}
					//concnt[this->shmIdx]++;////////////////////
					return;
				}
				else{
					memcpy(temp, this->shmName, sizeof(this->shmName));
					PRINTLOG(ERROR, "%s shmState has been abnormal as %d", temp, this->shmState);
					this->shmState = UNBORN;
					break;
				}
			}
			else{
				if( FALSE != ret ){
					this->shmkitError = TRUE;
					PRINTLOG(ERROR, "sys api error constructor will return");
					return;
				}
			}


			ret = createShm(*this);
			if(ret==SUCCESS){
				ret = addIntoShmKL(*this); ///////////////////////////////////
				if(ret != SUCCESS){
					this->shmState = SICK;
					PRINTLOG(ERROR, "Unable to addIntoShmKL()");
					break;
				}
				//this->shmIdx = checkConcnt();/////////////
				//concnt[this->shmIdx] = 1; ////////////////////////
				return;
			}
			else{
				this->shmState = UNBORN;
				PRINTLOG(ERROR, "Unable to createShm()");
				break;
			}

		}while(false);

		if(this->shmState == SICK){
			//concnt[this->shmIdx] = 0;//////////
			ret = deleteFromShmKL( *this );
			if( ret != SUCCESS ){
				PRINTLOG(ERROR, "Fail to deleteFromShmKL()");
			}

			if(((ShmHead_t *)(this->shmAddr))->totalCnt<=0){
				ret = dieOutShm(*this);
				if(ret!=SUCCESS){
					PRINTLOG(ERROR, "Unable to dieOutShm()");
					this->shmState = ZOMBIA;
					//abort();
				}
			}
		}

		if(this->shmState!=HEALTH){
			PRINTLOG(DEBUG, "Construct Shmkit error");
			this->shmkitError = TRUE;
		}
	}

	Shmkits( const Shmkits & ano )//:statBuf(ano.statBuf),ipcKey(ano.ipcKey),shmId(ano.shmId),
			//shmSize(ano.shmSize),shmAddr(ano.shmAddr)/*,pos(ano.pos)*/,shmIdx(ano.shmIdx),shmState(ano.shmState)
	{
		int ret = 0;
		STATE state = ano.shmState;
		if( ano.shmState == HEALTH ){

			state = releaseShmkits();
			if(state!=DIE){
				PRINTLOG(ERROR, "Unable to release current resource by copy construct");
				return;
			}

			memcpy(this->shmName, ano.shmName, sizeof(shmName));
			this->shmId = ano.shmId;
			this->ipcKey = ano.ipcKey;
			this->shmSize = ano.shmSize;
			this->shmAddr = ano.shmAddr;
			this->usrAddr = ano.usrAddr;
			this->shmState = ano.shmState;
			this->shmCnted = FALSE;

			ret = addIntoShmKL(*this); ///////////////////////////////////
			if(ret != SUCCESS){
				this->shmState = UNBORN;
				this->shmkitError = TRUE;
				PRINTLOG(ERROR, "Unable to addIntoShmKL()");
				return;
			}
		}
		else{
			PRINTLOG(WARN, "The given another shmkits is not in health state");
			return;
		}
	}

	~Shmkits()
	{
		STATE ret = releaseShmkits();
		if(ret != DIE ){
			PRINTLOG(DEBUG, "Released one abnormal state Shmkits by releaseShmkits()" );
		}
	}


	STATE releaseShmkits()
	{
		int ret = 0;

		//if( ( this->shmState != STANDBY ) && ( this->shmState != DIE ) && (this->shmState != UNBORN )){
		if(this->shmState == HEALTH || this->shmState == SICK ){
			ret = deleteFromShmKL( *this );
			if( ret != SUCCESS ){
				PRINTLOG(ERROR, "Unable to deleteFromShmKL()");
				this->shmState = SICK;
				this->shmkitError = TRUE;
			}

			if(((ShmHead_t *)(this->shmAddr))->totalCnt<=0){
				ret = dieOutShm(*this);
				if(ret!=SUCCESS){
					PRINTLOG(ERROR, "Unable to dieOutShm()");
					this->shmState = ZOMBIA;
					this->shmkitError = TRUE;
				}
			}
		}

		if( this->shmkitError == FALSE )
			this->shmState = DIE;

		return this->shmState;
	}

	/*bingo*/
	static int createShmKLHeader()
	{
		int ret = -1;
		bool_t headerExisted = FALSE;
		key_t ipcKey = -1;
		ShmHead_t shmHeader = { NULL, NULL, SHM_HEADER_NAME, -1,-1, sizeof(struct shmid_ds),  HEALTH, 0, PTHREAD_RWLOCK_INITIALIZER };
		struct shmid_ds statBuf = {0};

		IPC_Fund tol(shmHeader.shmName, IPC_NAME_LEN, IPC_SHME, sizeof(struct shmid_ds));

		ipcKey = tol.Get_IPC_Fund().ipcKey;

		PRINTLOG(DEBUG, "the.ipcKey is %d", ipcKey);

		shmKLHeaderId = shmget( ipcKey, sizeof(struct shmid_ds) +sizeof(ShmHead_t), IPC_CREAT|IPC_EXCL|0666);
		if(shmKLHeaderId<0){
			if(errno == EEXIST){
				headerExisted = TRUE;
				PRINTLOG(DEBUG, "shmHeader existed");
				shmKLHeaderId = shmget( ipcKey, sizeof(struct shmid_ds) +sizeof(ShmHead_t), IPC_CREAT|0666);
			}
			else{
				PRINTLOG(ERROR, "shmget error for %s", strerror(errno));
				return SYS_ERROR;
			}
		}
		shmKLHeaderExisted = TRUE;
//PRINTLOG(ERROR, "TEST POS 00 %s", strerror(errno));

		shmKLHeaderPtr = (ShmHead_t  *)shmat( shmKLHeaderId, NULL, 0);
		if(-1 ==(long)(shmKLHeaderPtr)){
			PRINTLOG(ERROR, "shmat error for %s", strerror(errno));
			return SYS_ERROR;
		}

		if(!headerExisted){
			memcpy( shmKLHeaderPtr, &shmHeader, sizeof(shmHeader) );
			ret = pthread_rwlock_init(&(shmKLHeaderPtr->shmRWLock), NULL);
			if(ret != 0){
				PRINTLOG(ERROR, "pthread_rwlock_init() error for %s", strerror(errno));
				return SYS_ERROR;
			}
		}

		shmKLHeaderPtr->totalCnt++;
		shmKLHeaderPtr->ipcKey = ipcKey;
		shmKLHeaderPtr->shmId = shmKLHeaderId;
		//shmKLHeaderPtr->shmSize = sizeof(struct shmid_ds);
		//shmKLHeaderPtr->shmState = HEALTH;

//PRINTLOG(ERROR, "TEST POS 01 %s", strerror(errno));
		ret = shmctl( shmKLHeaderId, IPC_STAT, &statBuf );
		if( ret == -1){
			PRINTLOG(ERROR, "shmctl error for %s", strerror(errno));
			return SYS_ERROR;
		}
		memcpy(shmKLHeaderPtr->bookmark, &statBuf, sizeof(statBuf));
//PRINTLOG(ERROR, "TEST POS 02 %s", strerror(errno));
		return SUCCESS;
	}


	/*bingo*/
	static bool_t isInShmKL ( Shmkits & the )
	{
		int ret = 0;
		bool_t res = FALSE;
		ShmHead_t * ptrA = shmKLHeaderPtr;
		//ShmHead_t * ptrB = ptrA;

		ret = pthread_rwlock_rdlock(&(shmKLHeaderPtr->shmRWLock));
		if(ret!=0)
		{
			PRINTLOG(ERROR, "Fail to pthread_rwlock_rdlock()");
			return SYS_ERROR;
		}

		for( ; ptrA!=NULL; ptrA = ptrA->next)
		{
			if(!(memcmp(ptrA->shmName, the.shmName, sizeof(the.shmName)))){
				the.ipcKey = ptrA->ipcKey;
				the.shmAddr =(char *) ptrA;
				the.shmId = ptrA->shmId;
				the.shmState = ptrA->shmState;
				the.shmSize = ptrA->shmSize;		 /*what if shmSize is different? TODO*/
				the.usrAddr = ptrA->bookmark;
				//the = (Shmkits)(ptrA->bookmark);
				res = TRUE;
				break;
			}
		}

		ret = pthread_rwlock_unlock(&(shmKLHeaderPtr->shmRWLock));
		if(ret!=0)
		{
			PRINTLOG(ERROR, "Fail to pthread_rwlock_rdlock()");
			return SYS_ERROR;
		}

		return res;

	}

	/*bingo*/
	static int addIntoShmKL( Shmkits & the )///////////////
	{
//PRINTLOG(ERROR, "TEST POS 0310 %s", strerror(errno));
		int ret = 0;
		ShmHead_t * ptr = NULL;
		if(the.shmCnted==FALSE){  /*not new shm created,  just link in*/
			ret =connectShm(the);
			if(ret!=SUCCESS){
				PRINTLOG(ERROR, "Fail to connectShm");
				return ret;
			}

			ptr = (ShmHead_t *) the.shmAddr;
			ptr->totalCnt++;
			the.shmCnted = TRUE;
			PRINTLOG(DEBUG, "A new Shmkits inserted into shmKL");
			return SUCCESS;
		}

		/*new shm created*/
		ShmHead_t * ptrA = shmKLHeaderPtr;
		ShmHead_t * ptrB = NULL;

		ret = pthread_rwlock_wrlock(&(shmKLHeaderPtr->shmRWLock));
		if(ret!=0)
		{
			PRINTLOG(ERROR, "Fail to pthread_rwlock_rdlock()");
			return SYS_ERROR;
		}

		for( ; ptrA!=NULL; ptrA = ptrA->next)
		{
			if(ptrA->next)
				continue;
			/*Add current shm into list's tail position*/
			ptrB = ptrA;
			ptrA->next = (ShmHead_t*)the.shmAddr;
			ptrA = ptrA->next;
			ptrA->prv = ptrB;
			ptrA->next = NULL;
			ptrA->totalCnt ++;
			break;
		}

		ret = pthread_rwlock_unlock(&(shmKLHeaderPtr->shmRWLock));
		if(ret!=0)
		{
			PRINTLOG(ERROR, "Fail to pthread_rwlock_rdlock()");
			return SYS_ERROR;
		}

		//shmKL.push_back( the );
		PRINTLOG(DEBUG, "A new Shmkits inserted into shmKL");
		return SUCCESS;
	}


	static int deleteFromShmKL( Shmkits & the )///////////////////
	{
		int ret = -1;

		ShmHead_t * ptrA = shmKLHeaderPtr;
		ShmHead_t * ptrB = NULL;
		ShmHead_t * thePtr = (ShmHead_t *) the.shmAddr;
/*
		ret = disconnectShm(the);
		if(SUCCESS!=ret){
			PRINTLOG(ERROR, "Fail to disconnectShm()");
			return ret;
		}
*/
		thePtr->totalCnt--;
		the.shmCnted = FALSE;
		if(thePtr->totalCnt>0){
			PRINTLOG(NOTE, "Unnecessary to delete from shm list other object is attached thePtr->totalCnt is %d", thePtr->totalCnt);
			return SUCCESS;
		}
		if(thePtr->totalCnt<0){
			PRINTLOG(WARN, "One abnormal shm is deleted from list thePtr->totalCnt is %d", thePtr->totalCnt);
		}

		/*Final connected object and delete it from shm list*/
		ret = pthread_rwlock_wrlock(&(shmKLHeaderPtr->shmRWLock));
		if(ret!=0)
		{
			PRINTLOG(ERROR, "Fail to pthread_rwlock_wrlock()");
			return SYS_ERROR;
		}

		for(; ptrA!=NULL; ptrA = ptrA->next)
		{
			if(!memcmp(the.shmName, ptrA->shmName, sizeof(the.shmName))){

				//if the shm is the head of list
				if( ptrA == shmKLHeaderPtr ){

					ptrA = ptrA->next;
					ptrA->prv = NULL;
					shmKLHeaderPtr = ptrA;
					break;
				}

				//if the shm is the tail of list
				if(ptrA->next == NULL){

					ptrB = ptrA;
					ptrA = ptrA->prv;
					ptrA->next = NULL;
					ptrB->prv = NULL;
					break;
				}

				//if the shm is in the middle of list
				ptrB = ptrA;
				ptrA = ptrA->prv;
				ptrA->next = ptrB->next;
				ptrA = ptrA->next;
				ptrA->prv = ptrB->prv;
				break;
			}
		}

		ret = pthread_rwlock_unlock(&(shmKLHeaderPtr->shmRWLock));
		if(ret!=0)
		{
			PRINTLOG(ERROR, "Fail to pthread_rwlock_rdlock()");
			return SYS_ERROR;
		}

		return SUCCESS;
	}

	/*bingo*/
	static int createShm( Shmkits & the )
	{
		int ret = -1;
		ShmHead_t shmHeader = { NULL, NULL, {0} , -1, 0, 0, STANDBY, 0, PTHREAD_RWLOCK_INITIALIZER, {} };
		ShmHead_t * shmHeaderPtr = NULL;

		/*
		char keyword[IPC_KEY_WORD_LEN] = {0};
		IPC_T shmType = IPC_SHME;
		snprintf( keyword, sizeof(IPC_T), "%d", shmType);
		snprintf( keyword+sizeof(IPC_T), IPC_NAME_LEN, "%s", the.shmName );
		snprintf( keyword+sizeof(IPC_T) + IPC_NAME_LEN, IPC_KEY_WORD_LEN - sizeof(IPC_T) - IPC_NAME_LEN, "%lu", the.shmSize );
		PRINTLOG(DEBUG, "sizeof(IPC_T) is %d, the.shmName is %s, the.shmSize is %lu", sizeof(IPC_T), the.shmName, the.shmSize);
		PRINTLOG(DEBUG, "keyword is %s", keyword );

		the.ipcKey =  (key_t)getHashIndex( (const char *)keyword, IPC_KEY_WORD_LEN, MAX_IPC_NUM );
		*/

		IPC_Fund tol(the.shmName, IPC_NAME_LEN, IPC_SHME, the.shmSize);

		the.ipcKey = tol.Get_IPC_Fund().ipcKey;

		PRINTLOG(DEBUG, "the.ipcKey is %d", the.ipcKey);

		the.shmId = shmget( the.ipcKey, the.shmSize+sizeof(ShmHead_t), IPC_CREAT|IPC_EXCL|0666);
		if(the.shmId<0){
			PRINTLOG(ERROR, "shmget error for %s", strerror(errno));
			return SYS_ERROR;
		}
//PRINTLOG(ERROR, "TEST POS 00 %s", strerror(errno));

		the.shmAddr = (char *)shmat( the.shmId, NULL, 0);
		if(-1 ==(long)(the.shmAddr)){
			PRINTLOG(ERROR, "shmat error for %s", strerror(errno));
			return SYS_ERROR;
		}
		the.shmCnted = TRUE;

		//shmHeader.shmRWLock = PTHREAD_RWLOCK_INITIALIZER;
		//shmHeader.totalCnt = 0;
		memcpy( shmHeader.shmName, the.shmName, sizeof(the.shmName) );
		memcpy( the.shmAddr, &shmHeader, sizeof(shmHeader) );

		shmHeaderPtr = (ShmHead_t *)(the.shmAddr);
		shmHeaderPtr->ipcKey = the.ipcKey;
		shmHeaderPtr->shmId = the.shmId;
		shmHeaderPtr->shmSize = the.shmSize;
		shmHeaderPtr->shmState = the.shmState;

		ret = pthread_rwlock_init(&(shmHeaderPtr->shmRWLock), NULL);
		if(ret != 0){
			PRINTLOG(ERROR, "pthread_rwlock_init() error for %s", strerror(errno));
			return SYS_ERROR;
		}


		the.usrAddr = the.shmAddr + sizeof(shmHeader);

//PRINTLOG(ERROR, "TEST POS 01 %s", strerror(errno));
		ret = shmctl( the.shmId, IPC_STAT, &(the.statBuf) );
		if( ret == -1){
			PRINTLOG(ERROR, "shmctl error for %s", strerror(errno));
			return SYS_ERROR;
		}
//PRINTLOG(ERROR, "TEST POS 02 %s", strerror(errno));
		return SUCCESS;
	}

	/*bingo*/
	static int dieOutShm(Shmkits & the)
	{
		int ret = -1;

		ret = shmdt( the.shmAddr );
		if( -1 == ret ){
			PRINTLOG(ERROR, "shmdt() error for %s", strerror(errno));
			return SYS_ERROR;
		}

      ret = shmctl( the.shmId, IPC_RMID, NULL);
		if( -1 == ret ){
			PRINTLOG(ERROR, "shmctl() error for %s", strerror(errno));
			return SYS_ERROR;
		}

		the.shmId = -1;
		the.ipcKey = 0;
		the.shmSize = 0;
		the.shmAddr = NULL;
		the.usrAddr = NULL;
		the.shmCnted = FALSE;
		memset(&(the.statBuf), 0, sizeof(the.statBuf));
		PRINTLOG(DEBUG, "die out shm successfully");
		return SUCCESS;
	}

	/*bingo*/
	static int connectShm(Shmkits & the)
	{
		int ret = -1;

//PRINTLOG(ERROR, "TEST POS 00 %s", strerror(errno));
		the.shmAddr = (char *)shmat( the.shmId, NULL, 0);
		if(-1 ==(long)(the.shmAddr)){
			PRINTLOG(ERROR, "shmat error for %s", strerror(errno));
			return SYS_ERROR;
		}
//PRINTLOG(ERROR, "TEST POS 01 %s", strerror(errno));
		ret = shmctl( the.shmId, IPC_STAT, &(the.statBuf) );
		if( ret == -1){
			PRINTLOG(ERROR, "shmctl error for %s", strerror(errno));
			return SYS_ERROR;
		}

		return SUCCESS;
	}

	/*bingo*/
	static int disconnectShm(Shmkits & the)
	{
		int ret = shmdt( the.shmAddr );
		if(ret!=0){
			PRINTLOG(ERROR,"shmdt error for %s", strerror(errno));
			return SYS_ERROR;
		}

		ret = shmctl( the.shmId, IPC_STAT, &(the.statBuf) );
		if( ret == -1){
			PRINTLOG(ERROR, "shmctl error for %s", strerror(errno));
			return SYS_ERROR;
		}

		return SUCCESS;
	}

	bool operator==( const Shmkits & ano )
	{
		if(!memcmp(this->shmName, ano.shmName, sizeof(shmName)) && (this->shmAddr == ano.shmAddr)){
			return true;
		}
		else {
			return false;
		}
	}

	bool operator!=( const Shmkits & ano )
	{
		if(*this == ano ){
			return false;
		}
		else{
			return true;
		}
	}

	Shmkits operator=( const Shmkits & ano )
	{
		int ret = 0;
		STATE state = ano.shmState;
		if( this->shmState == HEALTH ){

			state = releaseShmkits();
			if(state!=DIE){
				this->shmkitError = TRUE;
				PRINTLOG(ERROR, "Unable to release current resource by copy construct");
				return *this;;
			}

			memcpy(this->shmName, ano.shmName, sizeof(shmName));
			this->shmId = ano.shmId;
			this->ipcKey = ano.ipcKey;
			this->shmSize = ano.shmSize;
			this->shmAddr = ano.shmAddr;
			this->usrAddr = ano.usrAddr;
			this->shmState = ano.shmState;
			this->shmCnted = FALSE;

			ret = addIntoShmKL(*this); ///////////////////////////////////
			if(ret != SUCCESS){
				this->shmState = UNBORN;
				this->shmkitError = TRUE;
				PRINTLOG(ERROR, "Unable to addIntoShmKL()");
				return *this;
			}
		}
		else{
			PRINTLOG(WARN, "The given another shmkits is not in health state");
		}

		return *this;
	}



};

char Shmkits::	shmKLHeaderName[IPC_NAME_LEN] = {0};
int Shmkits::shmKLHeaderId = -1;
ShmHead_t * Shmkits::shmKLHeaderPtr = NULL;
bool_t Shmkits::shmKLHeaderExisted = FALSE;


#endif

int main()
{

//	Shmkits shm0;			//create an empty Shmkits without valid share memory
//	Shmkits shm1("shm2");//if no existing shm named "shm1", create it, or get the connection
//PRINTLOG(ERROR, "TEST POS 10 %s", strerror(errno));
#if 0
	shm0 = shm1;			//release shm0's original connection, get the new connection to shm1's
	Shmkits shm2(shm0);	//create a new shmkits, if shm0 is empty, then shm2 is empty, if shm0 is valid, then shm2 get the connection
	Shmkits shm3 = shm2;
	shm3 == shm0;			//to see if shm0 and shm3 are connecting to one shm together

#endif
/*
	const unsigned char word[] = {"give me shm 0"};
	int ipckey = New_Uniq_Key(  word, sizeof(word) );
	if(ipckey>0){
		PRINTLOG(DEBUG, "new ipckey is %d", ipckey );
	}
	else{
		PRINTLOG(WARN, "new ipckey fails %d", ipckey);
	}
*/
	//IPC_Fund ipcP(SHM_HEADER_NAME , strlen(SHM_HEADER_NAME), IPC_SHME,SHM_SIZE_DEF );
	Shmkits tmpShm;
	Shmkits realShm("realShm");
	Shmkits shmx(realShm);

	if(shmx == realShm){
		tmpShm = shmx;
	}
	return 0;
}


