/*
 * GroupTable.h
 *
 *  Created on: 2013/10/16
 *      Author: skgchxngsxyz-opensuse
 */

#ifndef GROUPTABLE_H_
#define GROUPTABLE_H_

#include "ProcMan.h"

#define MAX_TABLE_SIZE 256

typedef struct {
	int procId;
	ProcConfig config;
} ProcInfo;

typedef struct {
	int groupId;
	GroupConfig config;
	ProcInfo **procInfoArray;
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

int addNewProcToGroup(GroupInfo *group, ProcConfig config, char **cmds);

#endif /* GROUPTABLE_H_ */
