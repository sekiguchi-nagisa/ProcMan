#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ProcMan.h"
#include "GroupTable.h"
#include "invoke.h"

int verifyGroup(GroupInfo *groupInfo)
{
	int i;
	for(i = 0; i < groupInfo->config.procNum; i++) {
		if(groupInfo->procInfoArray[i] == NULL) {
			fprintf(stderr, "verification error\n");
			return -1;
		}
	}
	return 0;
}

int invokeAllProcInGroup(GroupInfo *groupInfo)
{

	return 0;
}

