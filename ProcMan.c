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

int setRedirect(int groupId, int procIndex, int fd, RedirectConfig *config)
{
	if(fd >= 0 && fd < 3) {
		int index = fd;
		ProcInfo *procInfo = getProc(getGroup(groupTable, groupId), procIndex);
		if(procInfo == NULL) {
			return -1;
		}
		procInfo->rconfigs[index] = config;
		return 0;
	}
	fprintf(stderr, "invalid file descriptor: %d\n", fd);
	return -1;
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
		return "";
	}
	return groupInfo->outMessage == NULL ? "" : groupInfo->outMessage;
}

int getPID(int groupId, int procIndex)
{
	ProcInfo *procInfo = getProc(getGroup(groupTable, groupId), procIndex);
	return procInfo == NULL ? -1 : procInfo->pid;
}
