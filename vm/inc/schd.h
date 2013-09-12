#ifndef __SCHD_H__
#define __SCHD_H__

#include "vm_common.h"
#include "dthread.h"


//default schduler time,unit in ms
#define SCHEDULER_TIMER     (20)

#define CAN_SCHEDULE()      (schedulerFlag==1)
#define SET_SCHEDULE()      do{schedulerFlag =1;}while(0)
#define CLR_SCHEDULE()      do{schedulerFlag =0;}while(0)
extern volatile u1 schedulerFlag;



void Schd_InitThreadLists(void);

void Schd_FinalThreadLists(void);

void Schd_ChangeThreadState(Thread * thread,THREAD_STATE_E newState);

/*must put READY STATE thread to the list head*/
void Schd_PushToReadyListHead(Thread * thread);

/*must put READY STATE thread to the list tail*/
void Schd_PushToReadyListTail(Thread * thread);

/*get next READY thread from ready list head*/
Thread * Schd_PopFromReadyList(void);

/*push non-READY thread to other list*/
void Schd_PushToOtherList(Thread * thread);

/*get READY thread,one per pop,get untill return null*/
Thread * Schd_PopReadyFromOtherList(void);

void Schd_DecSleepTime(u4 deltaTime);

/*delete and free DEAD state thread*/
void Schd_DelDeadThread(void);

/*calc ready thread account*/
int Schd_ReadyThreadAccount(void);

/*calc other thread account,if don't want to put *
 *dead thread into account,call Schd_DelDeadThread() in advance */
int Schd_OtherThreadAccount(void);

/* tatal thread account,but dead ones exclude*/
int Schd_ThreadAccountInTotal(void);

Thread * Schd_FindThreadByJavaObj(Object * javaObj);

void Schd_SCHEDULER(void);

#endif