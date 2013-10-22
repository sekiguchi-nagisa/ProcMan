#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ProcMan.h"
#include "verify.h"
#include "GlobalContext.h"

int verifyGroup(GroupInfo *groupInfo)
{
	if(groupInfo == NULL) {
		fprintf(stderr, "empty groupInfo\n");
		return -1;
	}
	int i;
	for(i = 0; i < groupInfo->config.procNum; i++) {
		ProcInfo *procInfo = getProcInfo(groupInfo, i);
		if(procInfo == NULL || verifyProc(procInfo) == -1) {
			fprintf(stderr, "groupInfo verification error\n");
			return -1;
		}
	}
	return 0;
}

int verifyProc(ProcInfo *procInfo)
{
	if(procInfo == NULL) {
		fprintf(stderr, "empty procInfo\n");
		return -1;
	}
	return 0;
}

int verifyRedirConfig(int fd, RedirConfig *config)
{
	if(config == NULL) {
		fprintf(stderr, "empty redirConfig\n");
		return -1;
	}
	// check target type
	int targetType = config->targetType;
	switch(targetType) {
	case NO_TARGET:
	case FILE_TARGET:
	case FD_TARGET: break;
	default:
		fprintf(stderr, "redirConfig verification error\n");
		fprintf(stderr, "---> invalid target type: %d\n", targetType);
		return -1;
	}
	// check append
	int append = config->append;
	switch(append) {
	case DISABLE:
	case ENABLE: break;
	default:
		fprintf(stderr, "redirConfig verification error\n");
		fprintf(stderr, "---> invalid append flag: %d\n", append);
		return -1;
	}
	// check fd
	int targetFd = config->fd;
	switch(targetFd) {
	case STDIN_FILENO:
		if(fd != STDIN_FILENO && targetType == FD_TARGET) {
			fprintf(stderr, "redirConfig verification error\n");
			fprintf(stderr, "---> invalid config\n");
			return -1;
		}
		break;
	case STDOUT_FILENO: break;
	case STDERR_FILENO: break;
	default:
		fprintf(stderr, "redirConfig verification error\n");
		fprintf(stderr, "---> invalid file descriptor: %d\n", targetFd);
		return -1;
	}
	// check fileName
	char *fileName = config->fileName;
	if(fd == 0 && fileName == NULL) {
		fprintf(stderr, "redirConfig verification error\n");
		fprintf(stderr, "---> empty input file name\n");
		return -1;
	}
	if(targetType == FILE_TARGET && fileName == NULL) {
		fprintf(stderr, "redirConfig verification error\n");
		fprintf(stderr, "---> empty output file name\n");
		return -1;
	}
	return 0;
}

int verifyGroupConfig(GroupConfig *config)
{
	if(config == NULL) {
		fprintf(stderr, "empty groupConfig\n");
		return -1;
	}
	// check procNum
	int procNum = config->procNum;
	if(procNum <= 0) {
		fprintf(stderr, "groupConfig verification error\n");
		fprintf(stderr, "---> invalid proc num: %d\n", procNum);
		return -1;
	}
	// check invokeType
	int invokeType = config->invokeType;
	switch(invokeType) {
	case SYNC_INVOKE: break;
	case ASYNC_INVOKE: break;
	default:
		fprintf(stderr, "groupConfig verification error\n");
		fprintf(stderr, "---> invalid invocation type: %d\n", invokeType);
		return -1;
	}
	// check mstgRedir
	int msgRedir = config->msgRedir;
	switch(msgRedir) {
	case DISABLE: break;
	case ENABLE: break;
	default:
		fprintf(stderr, "groupConfig verification error\n");
		fprintf(stderr, "---> invalid msgRedir flag: %d\n", msgRedir);
		return -1;
	}
	// check timeout
	int timeout = config->timeout;
	if(timeout < 0) {
		fprintf(stderr, "groupConfig verification error\n");
		fprintf(stderr, "---> invalid timeout setting: %d\n", timeout);
		return -1;
	}
	return 0;
}
