#ifndef PROCMAN_H_
#define PROCMAN_H_

// process invoke type
#define SYNC_INVOKE  0
#define ASYNC_INVOKE 1

typedef struct {
	int procNum;
	int invokeType;
	// TODO: callback function
} GroupConfig;

typedef struct {
	int cmdNum;
} ProcConfig;


int initContext();
int createProcGroup(GroupConfig config);
int addProcToGroup(int groupId, ProcConfig config, char **cmds);
int invokeAll(int groupId);
int deleteProcGroup(int groupId);

int getExitStatus(int groupId);
int getExitStatusAt(int groupId, int procId);
char *getOutMessage(int groupId);

#endif /* PROCMAN_H_ */
