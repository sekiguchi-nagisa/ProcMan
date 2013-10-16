#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GroupTable.h"
#include "ProcMan.h"
#include "tools.h"

#define GEN_INDEX(id) ((id - 1) % MAX_TABLE_SIZE)

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
	static int groupId = 0;
	GroupInfo *groupInfo = (GroupInfo *)malloc(sizeof(GroupInfo));
	CHECK_ALLOCATION(groupInfo);
	groupInfo->groupId = ++groupId;
	groupInfo->config = config;
	groupInfo->procInfoArray = (ProcInfo **)malloc(sizeof(ProcInfo *) * groupInfo->config.procNum);
	CHECK_ALLOCATION(groupInfo->procInfoArray);

	struct __table_entry * curEntry = table->entrys[GEN_INDEX(groupId)];
	while(curEntry != NULL) {
		curEntry = curEntry->nextEntry;
	}
	curEntry = (struct __table_entry *)malloc(sizeof(struct __table_entry));
	CHECK_ALLOCATION(curEntry);
	curEntry->groupInfo = groupInfo;
	curEntry->nextEntry = NULL;
	return groupInfo->groupId;
}

GroupInfo *getGroup(GroupTable *table, int groupId)
{
	struct __table_entry * curEntry = table->entrys[GEN_INDEX(groupId)];
	while(curEntry != NULL) {
		if(curEntry->groupInfo->groupId == groupId) {
			return curEntry->groupInfo;
		}
		curEntry = curEntry->nextEntry;
	}
	fprintf(stderr, "invalid group id: %d\n", groupId);
	return NULL;
}

int addNewProcToGroup(GroupInfo *group, ProcConfig config, char **cmds)
{
	return 0;
}
