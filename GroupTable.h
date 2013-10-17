#ifndef GROUPTABLE_H_
#define GROUPTABLE_H_

#include "ProcMan.h"

#define MAX_TABLE_SIZE 256
#define MAX_CMD_SIZE 256

// exit type
#define NORMAL_EXIT 0
#define INTR_EXIT 1

typedef struct {
	int procId;
	int exitType;
	int exitStatus;
	char **cmds;
	ProcConfig config;
} ProcInfo;

typedef struct {
	int groupId;
	GroupConfig config;
	ProcInfo **procInfoArray;
	char *outMessage;
} GroupInfo;

typedef struct {
	struct __table_entry {
		GroupInfo *groupInfo;
		struct __table_entry *nextEntry;
	} *entrys[MAX_TABLE_SIZE];
} GroupTable;

GroupTable *createGroupTable();

int addNewGroupToTable(GroupTable *table, GroupConfig config);
GroupInfo *getGroup(GroupTable *table, int groupId);
int deleteGroupFromTable(GroupTable *table, int groupId);

int addNewProcToGroup(GroupInfo *group, ProcConfig config, char **cmds);
ProcInfo *getProc(GroupInfo *group, int procId);

#endif /* GROUPTABLE_H_ */
