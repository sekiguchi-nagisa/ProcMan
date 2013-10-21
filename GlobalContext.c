#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GlobalContext.h"
#include "ProcMan.h"
#include "verify.h"

#define GEN_GRUOP_INDEX(id) ((id - 1) % MAX_TABLE_SIZE)

GlobalContext *createGlobalContext()
{
	GlobalContext *context = (GlobalContext *)malloc(sizeof(GlobalContext));
	CHECK_ALLOCATION(context);
	int i;
	for(i = 0; i < MAX_TABLE_SIZE; i++) {
		context->entrys[i] = NULL;
	}
	return context;
}

int addNewGroupToTable(GlobalContext *context, GroupConfig config)
{
	if(context == NULL) {
		fprintf(stderr, "GroupTable is NULL\n");
		return -1;
	}

	static int count = 0;
	int i;
	GroupInfo *groupInfo = (GroupInfo *)malloc(sizeof(GroupInfo));
	CHECK_ALLOCATION(groupInfo);
	groupInfo->groupId = ++count;
	groupInfo->config = config;
	groupInfo->procInfoArray = (ProcInfo **)malloc(sizeof(ProcInfo *) * groupInfo->config.procNum);
	groupInfo->outMessage = NULL;
	groupInfo->handler = NULL;
	CHECK_ALLOCATION(groupInfo->procInfoArray);
	for(i = 0; i < groupInfo->config.procNum; i++) {
		groupInfo->procInfoArray[i] = NULL;
	}

	int index = GEN_GRUOP_INDEX(groupInfo->groupId);
	struct __table_entry *newEntry = (struct __table_entry *)malloc(sizeof(struct __table_entry));
	CHECK_ALLOCATION(newEntry);
	newEntry->groupInfo = groupInfo;
	newEntry->nextEntry = NULL;

	if(context->entrys[index] == NULL) {
		context->entrys[index] = newEntry;
	} else {
		struct __table_entry *curEntry = context->entrys[index];
		while(curEntry->nextEntry != NULL) {
			curEntry = curEntry->nextEntry;
		}
		curEntry->nextEntry = newEntry;
	}
	return groupInfo->groupId;
}

GroupInfo *getGroupInfo(GlobalContext *context, int groupId)
{
	if(context == NULL) {
		fprintf(stderr, "GroupTable is NULL\n");
		return NULL;
	}

	struct __table_entry *curEntry = context->entrys[GEN_GRUOP_INDEX(groupId)];
	while(curEntry != NULL) {
		if(curEntry->groupInfo->groupId == groupId) {
			return curEntry->groupInfo;
		}
		curEntry = curEntry->nextEntry;
	}
	fprintf(stderr, "invalid group id: %d\n", groupId);
	return NULL;
}

int addExitHandlerToGroup(GroupInfo *groupInfo, ExitHandler handler)
{
	if(groupInfo == NULL) {
		fprintf(stderr, "GroupInfo is NULL\n");
		return -1;
	}
	if(handler == NULL) {
		fprintf(stderr, "ExitHandler is NULL\n");
		return -1;
	}

	if(groupInfo->config.invokeType != ASYNC_INVOKE) {
		fprintf(stderr, "invocation type is sync\n");
		return -1;
	}
	groupInfo->handler = handler;
	return 0;
}

int deleteGroupFromTable(GlobalContext *context, int groupId)
{
	if(context == NULL) {
		fprintf(stderr, "GroupTable is NULL\n");
		return -1;
	}

	int index = GEN_GRUOP_INDEX(groupId);
	struct __table_entry *curEntry = context->entrys[index];
	struct __table_entry *prevEntry = NULL;
	while(curEntry != NULL) {
		if(curEntry->groupInfo->groupId == groupId) {
			if(prevEntry == NULL) {
				context->entrys[index] = curEntry->nextEntry;
			} else {
				prevEntry->nextEntry = curEntry->nextEntry;
			}
			free(curEntry);
			return groupId;
		}
		prevEntry = curEntry;
		curEntry = curEntry->nextEntry;
	}
	fprintf(stderr, "invalid group id: %d\n", groupId);
	return -1;
}

int addNewProcToGroup(GroupInfo *groupInfo, int cmdNum, char **cmds)
{
	if(groupInfo == NULL) {
		fprintf(stderr, "GroupInfo is NULL\n");
		return -1;
	}

	int i;
	ProcInfo *procInfo = (ProcInfo *)malloc(sizeof(ProcInfo));
	CHECK_ALLOCATION(procInfo);
	procInfo->pid = -1;
	procInfo->exitType = -1;
	procInfo->cmdNum = cmdNum;
	procInfo->cmds = (char **)malloc(sizeof(char *) * (procInfo->cmdNum + 1));
	CHECK_ALLOCATION(procInfo->cmds);
	for(i = 0; i < 3; i++) {
		procInfo->rconfigs[i] = NULL;
	}
	for(i = 0; i < procInfo->cmdNum; i++) {
		procInfo->cmds[i] = (char *)malloc(sizeof(char) * MAX_CMD_SIZE);
		CHECK_ALLOCATION(procInfo->cmds[i]);
		strncpy(procInfo->cmds[i], cmds[i], MAX_CMD_SIZE);
	}
	procInfo->cmds[procInfo->cmdNum] = NULL;

	for(i = 0; i < groupInfo->config.procNum; i++) {
		if(groupInfo->procInfoArray[i] == NULL) {
			int index = i;
			groupInfo->procInfoArray[i] = (ProcInfo *)malloc(sizeof(ProcInfo));
			CHECK_ALLOCATION(groupInfo->procInfoArray[i]);
			groupInfo->procInfoArray[i] = procInfo;
			return index;
		}
	}
	free(procInfo);
	fprintf(stderr, "adding process failed!!\n");
	return -1;
}

ProcInfo *getProcInfo(GroupInfo *groupInfo, int procIndex)
{
	if(groupInfo == NULL) {
		fprintf(stderr, "GroupInfo is NULL\n");
		return NULL;
	}

	if(procIndex >= 0 && procIndex < groupInfo->config.procNum) {
		ProcInfo *procInfo = groupInfo->procInfoArray[procIndex];
		if(procInfo != NULL) {
			return procInfo;
		}
	}
	fprintf(stderr, "invalid procIndex index: %d\n", procIndex);
	return NULL;
}

int addRedirConfigToProc(ProcInfo *procInfo, int fd, RedirConfig *config)
{
	if(fd >= 0 && fd < 3) {
		int index = fd;
		if(procInfo == NULL) {
			return -1;
		}
		if(verifyRedirConfig(fd, config) == -1) {
			return -1;
		}
		procInfo->rconfigs[index] = config;
		return 0;
	}
	fprintf(stderr, "invalid file descriptor: %d\n", fd);
	return -1;
}
