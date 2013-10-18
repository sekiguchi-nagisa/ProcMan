#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ProcMan.h"
#include "GroupTable.h"
#include "invoke.h"

#define READ_PIPE  0
#define WRITE_PIPE 1

static void closeAllPipe(int arraySize, int pipefdArray[][2])
{
	int i;
	for(i = 0; i < arraySize; i++) {
		close(pipefdArray[i][0]);
		close(pipefdArray[i][1]);
	}
}

int verifyGroup(GroupInfo *groupInfo)
{
	int i;
	for(i = 0; i < groupInfo->config.procNum; i++) {
		if(groupInfo->procInfoArray[i] == NULL) {
			fprintf(stderr, "verification error\n");
			return -1;
		}
	}
	return 0;
}

int invokeAllProcInGroup(GroupInfo *groupInfo)
{
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
		closeAllPipe(procNum, pipefdArray);
		if(groupInfo->config.invokeType == SYNC_INVOKE) {
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
		} else {
			// TODO: asyn invoke
		}
		return 0;
	} else if(pid[i] == 0) {	// child process
		ProcInfo *procInfo = groupInfo->procInfoArray[i];
		if(i == 0) { // first proc
			if(procInfo->rconfigs[0] != NULL) {	// in redirect
				char *fileName = procInfo->rconfigs[0]->fileName;
				FILE *fp;
				if(fileName != NULL && (fp = fopen(fileName, "rb")) != NULL) {
					int fd = fileno(fp);
					dup2(fd, STDIN_FILENO);
					fclose(fp);
				}
			}
			if(procNum > 1) {
				dup2(pipefdArray[i][WRITE_PIPE], STDOUT_FILENO);
			}
		}
		if(i > 0 && i < procNum - 1) { // other proc
			dup2(pipefdArray[i - 1][READ_PIPE], STDIN_FILENO);
			dup2(pipefdArray[i][WRITE_PIPE], STDOUT_FILENO);
		}
		if(i == procNum - 1) { // last proc
			if(procNum > 1) {
				dup2(pipefdArray[i - 1][READ_PIPE], STDIN_FILENO);
			}
			// out & err redirect
			int i;
			for(i = 1; i < 3; i++) {	//TODO: other fd
				int outFileNo = i;
				RedirectConfig *rconfig = procInfo->rconfigs[i];
				if(rconfig == NULL) {
					continue;
				}
				if(rconfig->targetType == FILE_TARGET) {	// redirect to file
					FILE *fp;
					char *mode = (rconfig->append == ENABLE ? "ab" : "wb");
					if(rconfig->fileName != NULL &&
							(fp = fopen(rconfig->fileName, mode)) != NULL) {
						int fd = fileno(fp);
						dup2(fd, outFileNo);
						fclose(fp);
					}
				} else if(rconfig->targetType == FD_TARGET) {	// redirect to fd
					int fd = rconfig->fd;
					if(abs(outFileNo - fd) == 1) {
						dup2(fd, outFileNo);
					}
				}
			}
			if(groupInfo->config.msgRedir == ENABLE) {	// msg redirect

			}
		}
		closeAllPipe(procNum, pipefdArray);
		execv(procInfo->cmds[0], procInfo->cmds);
		perror("execution error");
		fprintf(stderr, "executed cmd: %s\n", procInfo->cmds[0]);
		return -1;
	} else {
		perror("child process error");
		return -1;
	}
	return 0;
}

