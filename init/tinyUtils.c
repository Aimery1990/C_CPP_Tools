/*
 * tinyUtils.c
 *
 *  Created on: May 5, 2018
 *      Author: aimery
 */
#include<stdio.h>
#include<stdlib.h>
#include"common.h"

int findZeroPos( const char * src,  unsigned srcSize )
{
	int i = 0;
	while( i< srcSize)
	{
		if(src[i] == 0){
			PRINTLOG(DEBUG, "%d", i);
			return i;
		}
		i++;
	}
	PRINTLOG(DEBUG, "%d", i);
	return FALSE;
}
