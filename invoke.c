#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

#include "ProcMan.h"
#include "GlobalContext.h"
#include "invoke.h"
#include "verify.h"

#define READ_PIPE  0
#define WRITE_PIPE 1

#define BUFFER_SIZE 256

struct __bufferEntry {
	int index;
	int size;
	char *buf;
	struct __bufferEntry *nextEntry;
};

typedef struct {
	struct __bufferEntry *firstEntry;
	struct __bufferEntry *lastEntry;
} MessageBuffer;

typedef struct {
	int groupId;
	int procNum;
	int timeout;
	int *pids;
	struct __cmdsWrapper {
		int cmdNum;
		char **cmds;
	} *wrapperArray;
	ExitHandler handler;
} MonitorInfo;

static void closeAllPipe(int arraySize, int pipefdArray[][2])
{
	int i;
	for(i = 0; i < arraySize; i++) {
		close(pipefdArray[i][0]);
		close(pipefdArray[i][1]);
	}
}

static void redirectTo(RedirConfig *rconfig, int outFileNo)
{
	if(rconfig == NULL) {
		return;
	}
	if(rconfig->targetType == FILE_TARGET) {	// redirect to file
		FILE *fp;
		char *mode = (rconfig->append == ENABLE ? "ab" : "wb");
		if(rconfig->fileName != NULL) {
			if((fp = fopen(rconfig->fileName, mode)) != NULL) {
				int fd = fileno(fp);
				dup2(fd, outFileNo);
				fclose(fp);
			} else {
				perror("file open error at output redir");
				exit(1);
			}
		}
	} else if(rconfig->targetType == FD_TARGET) {	// redirect to fd
		int fd = rconfig->fd;
		if(abs(outFileNo - fd) == 1) {
			dup2(fd, outFileNo);
		}
	}
}

static MessageBuffer *createBuffer()
{
	MessageBuffer *bufferList = (MessageBuffer *)malloc(sizeof(MessageBuffer));
	bufferList->firstEntry = NULL;
	bufferList->lastEntry = NULL;
	return bufferList;
}

static void appendBuf(MessageBuffer *bufferList, char *buf, int index, int bufferSize)
{
	if(bufferList == NULL) {
		fprintf(stderr, "empty Message Buffer\n");
		return;
	}
	if(buf == NULL) {
		fprintf(stderr, "empty Buffer\n");
		return;
	}
	struct __bufferEntry *entry = (struct __bufferEntry *)malloc(sizeof(struct __bufferEntry));
	CHECK_ALLOCATION(entry);
	entry->index = index;
	entry->size = bufferSize;
	entry->buf = (char *)malloc(sizeof(char) * bufferSize);
	CHECK_ALLOCATION(entry->buf);
	int i;
	for(i = 0; i < bufferSize; i++) {
		entry->buf[i] = buf[i];
	}
	entry->nextEntry = NULL;
	if(bufferList->firstEntry == NULL) {
		bufferList->firstEntry = entry;
		bufferList->lastEntry = entry;
	} else {
		bufferList->lastEntry->nextEntry = entry;
		bufferList->lastEntry = entry;
	}
}

static char *convertToMessage(MessageBuffer *bufferList)
{
	if(bufferList == NULL) {
		fprintf(stderr, "empty Message Buffer\n");
		return NULL;
	}
	int entrySize = bufferList->lastEntry->index;
	char *message = (char *)malloc(sizeof(char) * BUFFER_SIZE * entrySize);
	CHECK_ALLOCATION(message);
	struct __bufferEntry *entry = bufferList->firstEntry;
	int index = 0;
	while(entry != NULL) {
		int i;
		int size = entry->size;
		for(i = 0; i < size; i++) {
			message[index] = entry->buf[i];
			index++;
		}
		entry = entry->nextEntry;
	}
	free(bufferList);
	return message;
}

static MonitorInfo *createMonitorInfo(GroupInfo *groupInfo)
{
	MonitorInfo *monitorInfo = (MonitorInfo *)malloc(sizeof(MonitorInfo));
	CHECK_ALLOCATION(monitorInfo);
	monitorInfo->groupId = groupInfo->groupId;
	monitorInfo->procNum = groupInfo->config.procNum;
	monitorInfo->timeout = groupInfo->config.timeout;
	int procNum = monitorInfo->procNum;
	monitorInfo->pids = (int *)malloc(sizeof(int) * procNum);
	CHECK_ALLOCATION(monitorInfo->pids);
	monitorInfo->wrapperArray = (struct __cmdsWrapper *)malloc(sizeof(struct __cmdsWrapper) * procNum);
	CHECK_ALLOCATION(monitorInfo->wrapperArray);
	int i;
	for(i = 0; i < procNum; i++) {
		ProcInfo *procInfo = getProcInfo(groupInfo, i);
		monitorInfo->pids[i] = procInfo->pid;
		monitorInfo->wrapperArray[i].cmdNum = procInfo->cmdNum;
		int cmdNum = monitorInfo->wrapperArray[i].cmdNum;
		monitorInfo->wrapperArray[i].cmds = (char **)malloc(sizeof(char *) * cmdNum);
		CHECK_ALLOCATION(monitorInfo->wrapperArray[i].cmds);
		char **cmds = monitorInfo->wrapperArray[i].cmds;
		int j;
		for(j = 0; j < cmdNum; j++) {
			cmds[j] = (char *)malloc(sizeof(char) * 256);
			CHECK_ALLOCATION(cmds[j]);
			strncpy(cmds[j], procInfo->cmds[j], 256);
		}
	}
	monitorInfo->handler = groupInfo->handler;
	return monitorInfo;
}

void *monitorGroup(void *targetInfo)
{
	if(targetInfo == NULL) {
		fprintf(stderr, "empty targetInfo");
		return NULL;
	}
	MonitorInfo *info = (MonitorInfo *)targetInfo;

	// timeout
	int i;
	if(info->timeout > 0) {
		sleep(info->timeout);
		for(i = 0; i < info->procNum; i++) {
			kill(info->pids[i], SIGKILL);
		}
		fprintf(stderr, "process timeout\n");
		if(info->handler != NULL) {
			info->handler(ALARM_TERM);
		}
		free(info);
		return NULL;
	}

	// wait exit
	while(1) { //TODO:
		int count = 0;
		for(i = 0; i < info->procNum; i++) {
			int status;
			if(waitpid(info->pids[i], &status, WNOHANG) > 0) {
				count++;
			}
		}
		if(count == info->procNum) {
			if(info->handler != NULL) {
				info->handler(NORMAL_TERM);
			}
			free(info);
			return NULL;
		}
		sleep(1);
	}

	return NULL;
}

int invokeAllProcInGroup(GroupInfo *groupInfo)
{
	if(verifyGroup(groupInfo) == -1) {
		return -1;
	}
	int procNum = groupInfo->config.procNum;
	pid_t pid[procNum];
	int i = 0;
	int pipefdArray[procNum][2];

	for(i = 0; i < procNum; i++) {
		if(pipe(pipefdArray[i]) < 0) {
			perror("pipe creation error");
			return -1;
		}
	}
	for(i = 0; i < procNum && (pid[i] = fork()) > 0; i++) {
		groupInfo->procInfoArray[i]->pid = pid[i];
	}

	if(i == procNum) {	// parent process
		if(groupInfo->config.invokeType == SYNC_INVOKE) { // synchronous invocation
			closeAllPipe(procNum - 1, pipefdArray);
			close(pipefdArray[procNum - 1][WRITE_PIPE]);
			if(groupInfo->config.msgRedir == ENABLE &&
					groupInfo->procInfoArray[procNum - 1]->rconfigs[STDOUT_FILENO] == NULL) {
				int fd = pipefdArray[procNum - 1][READ_PIPE];
				MessageBuffer *bufferList = createBuffer();
				char buf[BUFFER_SIZE];
				int size;
				int count = 0;
				while((size = read(fd, &buf, BUFFER_SIZE)) > 0) {
					appendBuf(bufferList, buf, ++count, size);
				}
				groupInfo->outMessage = convertToMessage(bufferList);
			}
			close(pipefdArray[procNum - 1][READ_PIPE]);

			// timeout
			if(groupInfo->config.timeout > 0) {
				sleep(groupInfo->config.timeout);
				int i;
				for(i = 0; i < groupInfo->config.procNum; i++) {
					kill(groupInfo->procInfoArray[i]->pid, SIGKILL);
				}
				if(groupInfo->handler != NULL) {
					groupInfo->handler(ALARM_TERM);
				}
				return 0;
			}
			// wait for exit
			for(i = 0; i < procNum; i++) {
				int status;
				ProcInfo *procInfo = groupInfo->procInfoArray[i];
				waitpid(pid[i], &status, 0);
				if(WIFEXITED(status)) {
					procInfo->exitType = NORMAL_EXIT;
					procInfo->exitStatus = WEXITSTATUS(status);
				}
				if(WIFSIGNALED(status)) {
					procInfo->exitType = INTR_EXIT;
					procInfo->exitStatus = WTERMSIG(status);
				}
			}
			if(groupInfo->handler != NULL) {
				groupInfo->handler(NORMAL_TERM);
			}
		} else {	// asynchronous invocation
			closeAllPipe(procNum, pipefdArray);
			pthread_t thread;
			MonitorInfo *monitorInfo = createMonitorInfo(groupInfo);
			int ret = pthread_create(&thread, NULL, monitorGroup, monitorInfo);
			if(ret != 0) {
				perror("thread creation failed");
				exit(1);
			}
			pthread_detach(thread);
		}
		return 0;
	} else if(pid[i] == 0) {	// child process
		ProcInfo *procInfo = groupInfo->procInfoArray[i];
		if(i == 0) { // first proc
			if(procInfo->rconfigs[0] != NULL) {	// in redirect
				char *fileName = procInfo->rconfigs[0]->fileName;
				FILE *fp;
				if(fileName != NULL) {
					if((fp = fopen(fileName, "rb")) != NULL) {
						int fd = fileno(fp);
						dup2(fd, STDIN_FILENO);
						fclose(fp);
					} else {
						perror("file open error at input redir");
						exit(1);
					}
				}
			}
			if(procNum > 1) {
				dup2(pipefdArray[i][WRITE_PIPE], STDOUT_FILENO);
			}
		}
		if(i > 0 && i < procNum - 1) { // other proc
			dup2(pipefdArray[i - 1][READ_PIPE], STDIN_FILENO);
			// err redirect
			redirectTo(procInfo->rconfigs[STDERR_FILENO], STDERR_FILENO);
			dup2(pipefdArray[i][WRITE_PIPE], STDOUT_FILENO);
		}
		if(i == procNum - 1) { // last proc
			if(procNum > 1) {
				dup2(pipefdArray[i - 1][READ_PIPE], STDIN_FILENO);
			}
			// out & err redirect
			redirectTo(procInfo->rconfigs[STDOUT_FILENO], STDOUT_FILENO);
			redirectTo(procInfo->rconfigs[STDERR_FILENO], STDERR_FILENO);
			// msg redirect
			if(groupInfo->config.invokeType == SYNC_INVOKE &&
					groupInfo->config.msgRedir == ENABLE && procInfo->rconfigs[STDOUT_FILENO] == NULL) {
				dup2(pipefdArray[i][WRITE_PIPE], STDOUT_FILENO);
			}
		}
		closeAllPipe(procNum, pipefdArray);
		execv(procInfo->cmds[0], procInfo->cmds);
		perror("execution error");
		fprintf(stderr, "executed cmd: %s\n", procInfo->cmds[0]);
		exit(1);
	} else {
		perror("child process error");
		return -1;
	}
}

