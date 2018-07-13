/*
 * File_Operation.h
 *
 *  Created on: Mar 11, 2018
 *      Author: aimery
 */

#ifndef FILE_OPERATION_H_
#define FILE_OPERATION_H_

typedef enum{
	IS_EXISTED,
	IS_READ,
	IS_WRITTEN,
	IS_EXECUTED
}ACCESS_T;

#ifdef __cplusplus__
extern  "C" {
#endif

extern int is_accessable_file(const char * filepath, ACCESS_T acs );
extern int new_empty_file(const char * filepath);
//extern int new_flag_file(const char * filepath);
extern int app_open_file(const char * filepath);
extern int read_open_file(const char * filepath);
extern int app_write_file(int fd, char * buffer, unsigned len);
extern int only_read_file(int fd, char * buffer, unsigned len);

#ifdef __cplusplus__
}
#endif

#endif /* FILE_OPERATION_H_ */
