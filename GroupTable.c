#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GroupTable.h"
#include "ProcMan.h"

#define GEN_GRUOP_INDEX(id) ((id - 1) % MAX_TABLE_SIZE)

GroupTable *createGroupTable()
{
	GroupTable *groupTable = (GroupTable *)malloc(sizeof(GroupTable));
	CHECK_ALLOCATION(groupTable);
	int i;
	for(i = 0; i < MAX_TABLE_SIZE; i++) {
		groupTable->entrys[i] = NULL;
	}
	return groupTable;
}

int addNewGroupToTable(GroupTable *table, GroupConfig config)
{
	if(table == NULL) {
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
	CHECK_ALLOCATION(groupInfo->procInfoArray);
	for(i = 0; i < groupInfo->config.procNum; i++) {
		groupInfo->procInfoArray[i] = NULL;
	}

	int index = GEN_GRUOP_INDEX(groupInfo->groupId);
	struct __table_entry *newEntry = (struct __table_entry *)malloc(sizeof(struct __table_entry));
	CHECK_ALLOCATION(newEntry);
	newEntry->groupInfo = groupInfo;
	newEntry->nextEntry = NULL;

	if(table->entrys[index] == NULL) {
		table->entrys[index] = newEntry;
	} else {
		struct __table_entry *curEntry = table->entrys[index];
		while(curEntry->nextEntry != NULL) {
			curEntry = curEntry->nextEntry;
		}
		curEntry->nextEntry = newEntry;
	}
	return groupInfo->groupId;
}

GroupInfo *getGroup(GroupTable *table, int groupId)
{
	if(table == NULL) {
		fprintf(stderr, "GroupTable is NULL\n");
		return NULL;
	}

	struct __table_entry *curEntry = table->entrys[GEN_GRUOP_INDEX(groupId)];
	while(curEntry != NULL) {
		if(curEntry->groupInfo->groupId == groupId) {
			return curEntry->groupInfo;
		}
		curEntry = curEntry->nextEntry;
	}
	fprintf(stderr, "invalid group id: %d\n", groupId);
	return NULL;
}

int deleteGroupFromTable(GroupTable *table, int groupId)
{
	if(table == NULL) {
		fprintf(stderr, "GroupTable is NULL\n");
		return -1;
	}

	int index = GEN_GRUOP_INDEX(groupId);
	struct __table_entry *curEntry = table->entrys[index];
	struct __table_entry *prevEntry = NULL;
	while(curEntry != NULL) {
		if(curEntry->groupInfo->groupId == groupId) {
			if(prevEntry == NULL) {
				table->entrys[index] = curEntry->nextEntry;
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

int addNewProcToGroup(GroupInfo *group, int cmdNum, char **cmds)
{
	if(group == NULL) {
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

	for(i = 0; i < group->config.procNum; i++) {
		if(group->procInfoArray[i] == NULL) {
			procInfo->procIndex = i;
			group->procInfoArray[i] = (ProcInfo *)malloc(sizeof(ProcInfo));
			CHECK_ALLOCATION(group->procInfoArray[i]);
			group->procInfoArray[i] = procInfo;
			return procInfo->procIndex;
		}
	}
	free(procInfo);
	fprintf(stderr, "adding process failed!!\n");
	return -1;
}

ProcInfo *getProc(GroupInfo *group, int procIndex)
{
	if(group == NULL) {
		fprintf(stderr, "GroupInfo is NULL\n");
		return NULL;
	}

	if(procIndex >= 0 && procIndex < group->config.procNum) {
		ProcInfo *procInfo = group->procInfoArray[procIndex];
		if(procInfo != NULL) {
			return procInfo;
		}
	}
	fprintf(stderr, "invalid procIndex index: %d\n", procIndex);
	return NULL;
}
