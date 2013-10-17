#ifndef TOOLS_H_
#define TOOLS_H_


#define CHECK_ALLOCATION(ptr) \
	do { \
		if(ptr == NULL) { \
			fprintf(stderr, "%s: %d ", __FILE__, __LINE__ ); \
			perror("Memory Allocation failed"); \
			exit(1); \
		} \
	} while(0)



#endif /* TOOLS_H_ */
