#ifndef GLOBALCONTEXT_H_
#define GLOBALCONTEXT_H_

#include "ProcMan.h"

#define MAX_TABLE_SIZE 256
#define MAX_CMD_SIZE 256

typedef struct {
	int pid;
	int exitType;
	int exitStatus;
	int cmdNum;
	char **cmds;
	RedirConfig *rconfigs[3];
} ProcInfo;

typedef struct {
	int groupId;
	GroupConfig config;
	ProcInfo **procInfoArray;
	char *outMessage;
	ExitHandler handler;
} GroupInfo;

typedef struct {
	struct __table_entry {
		GroupInfo *groupInfo;
		struct __table_entry *nextEntry;
	} *entrys[MAX_TABLE_SIZE];
} GlobalContext;

GlobalContext *createGlobalContext();

int addNewGroupToTable(GlobalContext *context, GroupConfig config);
GroupInfo *getGroupInfo(GlobalContext *context, int groupId);
int addExitHandlerToGroup(GroupInfo *groupInfo, ExitHandler handler);
int deleteGroupFromTable(GlobalContext *context, int groupId);

int addNewProcToGroup(GroupInfo *groupInfo, int cmdNum, char **cmds);
ProcInfo *getProcInfo(GroupInfo *groupInfo, int procIndex);
int addRedirConfigToProc(ProcInfo *procInfo, int fd, RedirConfig *config);

#endif /* GLOBALCONTEXT_H_ */
