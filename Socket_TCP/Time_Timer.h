/*
 * Time_Timer.h
 *
 *  Created on: Mar 12, 2018
 *      Author: aimery
 */

#ifndef TIME_TIMER_H_
#define TIME_TIMER_H_


#ifdef __cplusplus__
extern  "C" {
#endif


extern time_t get_unix_time_stamp();
extern clock_t get_program_useconds();
extern bool_t us_timer(long start, long useconds);



#ifdef __cplusplus__
}
#endif

#endif /* TIME_TIMER_H_ */
