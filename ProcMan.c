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
	return addNewGroupToTable(groupTable, config);
}

int addProcToGroup(int groupId, int cmdNum, char **cmds)
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	return addNewProcToGroup(groupInfo, cmdNum, cmds);
}

int setRedirect(int groupId, int procIndex, int fd, RedirConfig *config)
{
	ProcInfo *procInfo = getProc(getGroup(groupTable, groupId), procIndex);
	return addRedirConfigToProc(procInfo, fd, config);
}

int setExitHandler(int groupId, ExitHandler handler)
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	return addExitHandlerToGroup(groupInfo, handler);
}

int invokeAll(int groupId)
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	return invokeAllProcInGroup(groupInfo);
}

int deleteProcGroup(int groupId)
{
	return deleteGroupFromTable(groupTable, groupId);
}

int getExitStatus(int groupId)	//TODO: support signal exit
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	if(groupInfo == NULL) {
		return INVALID_STATUS;
	}
	ProcInfo *lastProc = getProc(groupInfo, groupInfo->config.procNum - 1);
	return lastProc == NULL ? INVALID_STATUS : lastProc->exitStatus;
}

int getExitStatusAt(int groupId, int procIndex)	//TODO: support signal exit
{
	ProcInfo *procInfo = getProc(getGroup(groupTable, groupId), procIndex);
	return procInfo == NULL ? INVALID_STATUS : procInfo->exitStatus;
}

char *getOutMessage(int groupId)
{
	GroupInfo *groupInfo = getGroup(groupTable, groupId);
	if(groupInfo == NULL) {
		return NULL;
	}
	return groupInfo->outMessage == NULL ? "" : groupInfo->outMessage;
}

int getPID(int groupId, int procIndex)
{
	ProcInfo *procInfo = getProc(getGroup(groupTable, groupId), procIndex);
	return procInfo == NULL ? -1 : procInfo->pid;
}
