#include <stdio.h>

#include "ProcMan.h"
#include "GroupTable.h"
#include "invoke.h"


static GroupTable *groupTable = NULL;

int initContext()
{
	groupTable = createGroupTable();
	return 0;
}

int createProcGroup(GroupConfig config)
{
	if(groupTable == NULL) {
		fprintf(stderr, "groupTable has not initialized yet\n");
		return -1;
	}
	return addNewGroupToTable(groupTable, config);
}

int addProcToGroup(int groupId, ProcConfig config, char **cmds)
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	return addNewProcToGroup(groupInfo, config, cmds);
}

int invokeAll(int groupId)
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	if(verifyGroup(groupInfo) == -1) {
		return -1;
	}
	return invokeAllProcInGroup(groupInfo);
}

int deleteProcGroup(int groupId)
{
	return deleteGroupFromTable(groupTable, groupId);
}

