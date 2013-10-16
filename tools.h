/*
 * tools.h
 *
 *  Created on: 2013/10/16
 *      Author: skgchxngsxyz-opensuse
 */

#ifndef TOOLS_H_
#define TOOLS_H_


#define CHECK_ALLOCATION(ptr) \
	do { \
		if(ptr == NULL) { \
			fprintf(stderr, "%s: %d ", __FILE__, __LINE__ ); \
			perror("Memory Allocation failed"); \
			exit(1); \
		} \
	} while(0)



#endif /* TOOLS_H_ */
