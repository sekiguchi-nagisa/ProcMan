#ifndef GROUPTABLE_H_
#define GROUPTABLE_H_

#include "ProcMan.h"

#define MAX_TABLE_SIZE 256
#define MAX_CMD_SIZE 256

typedef struct {
	int procIndex;
	int pid;
	int exitType;
	int exitStatus;
	int cmdNum;
	char **cmds;
	RedirectConfig *rconfigs[3];
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

int addNewProcToGroup(GroupInfo *group, int cmdNum, char **cmds);
ProcInfo *getProc(GroupInfo *group, int procIndex);

// ############# utils #################
#define CHECK_ALLOCATION(ptr) \
	do { \
		if(ptr == NULL) { \
			fprintf(stderr, "%s: %d ", __FILE__, __LINE__ ); \
			perror("Memory Allocation failed"); \
			exit(1); \
		} \
	} while(0)


#endif /* GROUPTABLE_H_ */
