#ifndef PROCMAN_H_
#define PROCMAN_H_

// process invoke type
#define SYNC_INVOKE  0
#define ASYNC_INVOKE 1

// exit type
#define NORMAL_EXIT 0
#define INTR_EXIT   1

// exit status
#define INVALID_STATUS 999

// flag
#define DISABLE 0
#define ENABLE  1

// redirect target type
#define NO_TARGET   0
#define FILE_TARGET 1
#define FD_TARGET   2

typedef struct {
	int targetType;
	int append;
	int fd;
	char *fileName;
} RedirConfig;

typedef struct {
	int procNum;
	int invokeType;
	int msgRedir;	// stdout to variable
	long timeout;
	// TODO: callback function
} GroupConfig;

int initContext();
int createProcGroup(GroupConfig config);
int addProcToGroup(int groupId, int cmdNum, char **cmds);
int setRedirect(int groupId, int procIndex, int fd, RedirConfig *config);
int invokeAll(int groupId);
int deleteProcGroup(int groupId);

int getExitStatus(int groupId);
int getExitStatusAt(int groupId, int procIndex);
char *getOutMessage(int groupId);
int getPID(int groupId, int procIndex);

#endif /* PROCMAN_H_ */
