#ifndef PROCMAN_H_
#define PROCMAN_H_

// process run type
#define SYNC_RUN = 0;
#define ASYNC_RUN = 1;


typedef struct {
	int cmdNum;
} ProcConfig;

typedef struct {
	int procNum;
	int runType;
	// TODO: callback function
} GroupConfig;


int initContext();
int createProcGroup(GroupConfig config);
int addProcToGroup(int groupId, ProcConfig config, char **cmds);
int invokeAll(int groupId);
int deleteProcGroup(int groupId);

#endif /* PROCMAN_H_ */
