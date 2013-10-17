#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GroupTable.h"
#include "ProcMan.h"
#include "tools.h"

#define GEN_GRUOP_INDEX(id) ((id - 1) % MAX_TABLE_SIZE)
#define GEN_PROC_INDEX(id) (id - 1)

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
	static int count = 0;
	int i;
	GroupInfo *groupInfo = (GroupInfo *)malloc(sizeof(GroupInfo));
	CHECK_ALLOCATION(groupInfo);
	groupInfo->groupId = ++count;
	groupInfo->config = config;
	groupInfo->procInfoArray = (ProcInfo **)malloc(sizeof(ProcInfo *) * groupInfo->config.procNum);
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

int addNewProcToGroup(GroupInfo *group, ProcConfig config, char **cmds)
{
	static int count = 0;
	int i;
	ProcInfo *procInfo = (ProcInfo *)malloc(sizeof(ProcInfo));
	CHECK_ALLOCATION(procInfo);
	procInfo->procId = ++count;
	procInfo->exitType = -1;
	procInfo->config = config;
	procInfo->cmds = (char **)malloc(sizeof(char *) * (procInfo->config.cmdNum + 1));
	CHECK_ALLOCATION(procInfo->cmds);
	for(i = 0; i < procInfo->config.cmdNum; i++) {
		procInfo->cmds[i] = (char *)malloc(sizeof(char) * MAX_CMD_SIZE);
		CHECK_ALLOCATION(procInfo->cmds[i]);
		strncpy(procInfo->cmds[i], cmds[i], MAX_CMD_SIZE);
	}
	procInfo->cmds[procInfo->config.cmdNum] = NULL;

	for(i = 0; i < group->config.procNum; i++) {
		if(group->procInfoArray[i] == NULL) {
			group->procInfoArray[i] = (ProcInfo *)malloc(sizeof(ProcInfo));
			CHECK_ALLOCATION(group->procInfoArray[i]);
			group->procInfoArray[i] = procInfo;
			return procInfo->procId;
		}
	}
	free(procInfo);
	fprintf(stderr, "adding process failed!!\n");
	return -1;
}

ProcInfo *getProc(GroupInfo *group, int procId)
{
	int index = GEN_PROC_INDEX(procId);
	if(index >= 0 && index < group->config.procNum) {
		ProcInfo *procInfo = group->procInfoArray[index];
		if(procInfo != NULL) {
			return procInfo;
		}
	}
	fprintf(stderr, "invalid procId id: %d\n", procId);
	return NULL;
}
