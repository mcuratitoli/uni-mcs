#ifndef LIBSLU_H
#define LIBSLU_H

typedef unsigned int BOOL;

typedef struct 
{
	char* matrixFile;
	unsigned long rows;
	unsigned long cols; 
	unsigned long nonZero;
	double* solution;	
	unsigned int solutionTime;
	double relError;
	unsigned long memoryUsage;

} LIBSLU_SOLUTION;

typedef int LIBSLU_RET_CODE;

#define LIBSLU_NO_ERR								0
#define LIBSLU_ERR_PARAM							1
#define LIBSLU_ERR_MALLOC							2
#define LIBSLU_ERR_FOPEN							3
#define LIBSLU_ERR_MM_HEADER						4
#define LIBSLU_ERR_MM_BODY					        5
#define LIBSLU_ERR_MM_FILE						    6

LIBSLU_SOLUTION* libslu_init(char* matrixFile, LIBSLU_RET_CODE* error, BOOL debug);
LIBSLU_RET_CODE  libslu_solvesparse(LIBSLU_SOLUTION* solutionHdlr, BOOL debug);
LIBSLU_RET_CODE  libslu_print_solution(LIBSLU_SOLUTION* solutionHdlr, BOOL debug);
LIBSLU_RET_CODE  libslu_free(LIBSLU_SOLUTION* solutionHdlr, BOOL debug);

#endif /* LIBSLU_H */
