#include <stdio.h>

#include "ProcMan.h"
#include "GlobalContext.h"
#include "invoke.h"


static GlobalContext *context = NULL;

int initContext()
{
	context = createGlobalContext();
	return 0;
}

int createProcGroup(GroupConfig config)
{
	return addNewGroupToTable(context, config);
}

int addProcToGroup(int groupId, int cmdNum, char **cmds)
{
	GroupInfo *groupInfo = getGroupInfo(context, groupId);
	return addNewProcToGroup(groupInfo, cmdNum, cmds);
}

int setRedirect(int groupId, int procIndex, int fd, RedirConfig *config)
{
	ProcInfo *procInfo = getProcInfo(getGroupInfo(context, groupId), procIndex);
	return addRedirConfigToProc(procInfo, fd, config);
}

int setExitHandler(int groupId, ExitHandler handler)
{
	GroupInfo *groupInfo = getGroupInfo(context, groupId);
	return addExitHandlerToGroup(groupInfo, handler);
}

int invokeAll(int groupId)
{
	GroupInfo *groupInfo = getGroupInfo(context, groupId);
	return invokeAllProcInGroup(groupInfo);
}

int deleteProcGroup(int groupId)
{
	return deleteGroupFromTable(context, groupId);
}

int getExitStatus(int groupId)	//TODO: support signal exit
{
	GroupInfo *groupInfo = getGroupInfo(context, groupId);
	if(groupInfo == NULL) {
		return INVALID_STATUS;
	}
	ProcInfo *lastProc = getProcInfo(groupInfo, groupInfo->config.procNum - 1);
	return lastProc == NULL ? INVALID_STATUS : lastProc->exitStatus;
}

int getExitStatusAt(int groupId, int procIndex)	//TODO: support signal exit
{
	ProcInfo *procInfo = getProcInfo(getGroupInfo(context, groupId), procIndex);
	return procInfo == NULL ? INVALID_STATUS : procInfo->exitStatus;
}

char *getOutMessage(int groupId)
{
	GroupInfo *groupInfo = getGroupInfo(context, groupId);
	if(groupInfo == NULL) {
		return NULL;
	}
	return groupInfo->outMessage == NULL ? "" : groupInfo->outMessage;
}

int getPID(int groupId, int procIndex)
{
	ProcInfo *procInfo = getProcInfo(getGroupInfo(context, groupId), procIndex);
	return procInfo == NULL ? -1 : procInfo->pid;
}
