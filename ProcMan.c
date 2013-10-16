#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ProcMan.h"
#include "GroupTable.h"


static GroupTable *groupTable = NULL;

int initContext()
{
	groupTable = createGroupTable();
	return 0;
}

int createProcGroup(GroupConfig config)
{
	if(groupTable == NULL) {
		fprintf(stderr, "groupTable has not initialized yet");
		return -1;
	}
	return addNewGroupToTable(groupTable, config);
}

int addProcToGroup(int groupId, ProcConfig config, char **cmds)
{
	return 0;
}

int invokeAll(int groupId)
{
	return 0;
}

int deleteProcGroup(int groupId)
{
	return 0;
}
