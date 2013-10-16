/*
 * ProcMan.h
 *
 *  Created on: 2013/10/16
 *      Author: skgchxngsxyz-opensuse
 */

#ifndef PROCMAN_H_
#define PROCMAN_H_

typedef struct {

} ProcConfig;

typedef struct {
	int procNum;
} GroupConfig;


int initContext();
int createProcGroup(GroupConfig config);
int addProcToGroup(int groupId, ProcConfig config, char **cmds);
int invokeAll(int groupId);
int deleteProcGroup(int groupId);

#endif /* PROCMAN_H_ */
