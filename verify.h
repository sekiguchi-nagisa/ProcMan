#ifndef VERIFY_H_
#define VERIFY_H_

#include "ProcMan.h"
#include "GlobalContext.h"

#define CHECK_ALLOCATION(ptr) \
	do { \
		if(ptr == NULL) { \
			fprintf(stderr, "%s: %d ", __FILE__, __LINE__ ); \
			perror("Memory Allocation failed"); \
			exit(1); \
		} \
	} while(0)

int verifyGroup(GroupInfo *groupInfo);
int verifyProc(ProcInfo *procInfo);
int verifyRedirConfig(int fd, RedirConfig *config);
int verifyGroupConfig(GroupConfig *config);

#endif /* VERIFY_H_ */
