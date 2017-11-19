#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <slu_ddefs.h>

#ifdef WIN32
	#include <windows.h>
#elif __APPLE__
	#include <mach/mach.h>
	#include <mach/mach_time.h>
	#include <CoreServices/CoreServices.h>
#endif

#include "libslu.h"

/* #define LIBSLU_FILE_TEST */

double* libslu_matrix_vector(int m, int n, int ncc, int icc[], int ccc[], double acc[], double x[]);
LIBSLU_RET_CODE libslu_read_mm_file(FILE* fp, int* n, int* m, int* nonZero, double** nzVal, int** rowind, int** colptr);
void libslu_print_vector_file(char* filePath, double* solution, int solutionLen);

LIBSLU_SOLUTION* libslu_init(char* matrixFile, LIBSLU_RET_CODE* error, BOOL debug)
{
	LIBSLU_SOLUTION* libsluHdlr = NULL;	

	/*Verifica validità parametri */
	if (error == NULL)
	{
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "NULL parameter - error");			
		}
		return NULL;
	}

	if (matrixFile == NULL)
	{
		*error = LIBSLU_ERR_PARAM;
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "NULL parameter - matrixFile");			
		}
		return NULL;
	}

	/* Alloca la memoria necessaria per contenere una struttura LIBSLU_SOLUTION */
	libsluHdlr = (LIBSLU_SOLUTION*) malloc(sizeof(LIBSLU_SOLUTION));
	if (libsluHdlr == NULL)
	{
		*error = LIBSLU_ERR_MALLOC;
		/* Errore malloc */
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_MALLOC, "malloc error");
		}
		return NULL;
	}

	/* Inizializza la memoria */
	memset(libsluHdlr, 0, sizeof(LIBSLU_SOLUTION));
	/* Copia stringa nome file matrice */
	libsluHdlr->matrixFile = (char*) malloc(strlen(matrixFile) + 1);
	strcpy(libsluHdlr->matrixFile, matrixFile);
	if(libsluHdlr->matrixFile == NULL)
	{
		*error = LIBSLU_ERR_MALLOC;
		/* Errore malloc */
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_MALLOC, "malloc error");
		}
		goto error_free;
	}

	return libsluHdlr;

	error_free:

		if(libsluHdlr != NULL)
		{
			free(libsluHdlr);
		}

		return NULL;
}


LIBSLU_RET_CODE  libslu_solvesparse(LIBSLU_SOLUTION* solutionHdlr, BOOL debug)
{
	FILE* fp;							/* File pointer a file matrice (Formato HB)            */
	SuperMatrix A, L, U, B;				/* Matrici e vettori in formato superLU (SuperMatrix)  */
	int m,n;							/* m : numero righe, n : numero colonne 			   */
	int nonZero;						/* Numero di elementi diversi da 0 in A 			   */
	double* nzVal;						/* Puntatore a valori diversi da zero in A 			   */
	int* rowind;						/* Indice righe elementi in nzVal				       */
	int* colptr;						/* Indice elementi inizio colonne in nzVal		       */
	double* xe =  NULL;					/* Vettore unitario di lunghezza m 					   */
	double* b = NULL;					/* Vettore dei termini noti							   */	
	int* permRow = NULL;				/* Vettore permutazioni righe					       */
	int* permCol = NULL;				/* Vettore permutazioni colonne					       */
	superlu_options_t options;			/* Opzioni solver SuperLU  						       */
    SuperLUStat_t stat;					/* Statistiche solver SuperLU   			    	   */
	int info;
	int ret;
	int i = 0;

#ifdef WIN32
	LARGE_INTEGER StartingTime;			/* TIMESTAMP prima di invocare il solver     			 */
	LARGE_INTEGER EndingTime;			/* TIMESTAMP dopo aver invocato il solver				 */
	LARGE_INTEGER Frequency;			/* Tick per secondo CPU								     */
	LARGE_INTEGER ElapsedMicroseconds;	/* Tempo impiegato in microsecondi					     */
#elif __APPLE__
	unsigned long StartingTime;			/* TIMESTAMP prima di invocare il solver     			 */
	unsigned long EndingTime;			/* TIMESTAMP dopo aver invocato il solver				 */
	unsigned long Elapsed;				/* Differenza tra inizio e fine						     */
	unsigned long ElapsedNano;			/* Tempo impiegato in nanosecondi					     */
	static mach_timebase_info_data_t    sTimebaseInfo;
#endif


	/* Verifica validità parametri */
	if (solutionHdlr == NULL)
	{
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "NULL parameter - solutionHdlr");
		}
		return LIBSLU_ERR_PARAM;
	}

	if (solutionHdlr->matrixFile == NULL)
	{
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "NULL parameter - solutionHdlr->matrixFile");
		}
		return LIBSLU_ERR_PARAM;
	}

	/* Apre file matrice */
	fp = fopen(solutionHdlr->matrixFile, "rb");
	if (fp == NULL)
	{
		/* Errore apertura file */
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "fopen error");
		}
		return LIBSLU_ERR_FOPEN;
	}

	/* Converte file matrice da formato MM a formato interno CC */
	if (libslu_read_mm_file(fp,&n,&m,&nonZero,&nzVal,&rowind,&colptr) != 0)
	{
		fclose(fp);
		/* Errore lettura file file */
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_MM_FILE, "MM file read error");
		}
		return LIBSLU_ERR_MM_FILE;
	}

	/* Close file */
	fclose(fp);

	/* Popola una struttura SuperMatrix utilizzando A (rappresentazione interna SuperLU) */
    dCreate_CompCol_Matrix(&A, m, n, nonZero, nzVal, rowind, colptr, SLU_NC, SLU_D, SLU_GE);

    /* Costruisce vettore termini noti b = A * xe. xe è il vettore unitario di dimensione pari a m */
    /* Alloca xe */
    xe = (double*) malloc(sizeof(double) * m);
    if (xe == NULL)
    {
    	ret = LIBSLU_ERR_MALLOC;
    	if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_MALLOC, "malloc error");
		}
    	goto error_free;
    }
    /* Inizializza xe */
    for (i = 0; i < m; i++)
    {
    	xe[i] = 1.0;
    }
    /* Ricava b */
    b = libslu_matrix_vector(m, n, nonZero, rowind, colptr, nzVal, xe);

#ifdef LIBSLU_FILE_TEST
	//Scrive vettore termini noti in file di test
    libslu_print_vector_file("/Users/7iacura/Documents/mcs/testB.txt", b, m);
    printf("testB ");
#endif

    /* Converte B in formato interno SuperLU */
    dCreate_Dense_Matrix(&B, m, 1, b, m, SLU_DN, SLU_D, SLU_GE);

    /* Inizializza vettori permutazioni */
    permRow = (int*) malloc(sizeof(int) * m);
    if (permRow == NULL)
    {
    	ret = LIBSLU_ERR_MALLOC;
    	if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_MALLOC, "malloc error");
		}
    	goto error_free;
    }
    permCol = (int*) malloc(sizeof(int) * m);
    if (permCol == NULL)
    {
    	ret = LIBSLU_ERR_MALLOC;
    	if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_MALLOC, "malloc error");
		}
    	goto error_free;
    }

    /* Opzioni di default per SuperLU */
    set_default_options(&options);
    options.ColPerm = COLAMD;

    /* Inizializza statistiche */
    StatInit(&stat);

#ifdef WIN32
	/* Acquisce un timestamp ad alta risoluzione (WIN32 API)*/
	QueryPerformanceCounter(&StartingTime);
	/* Acquisisce frequenza CPU in tick per secondo			*/
	QueryPerformanceFrequency(&Frequency);
#elif __APPLE__
	/* Acquisce un timestamp in mach_absolute_time */
	StartingTime = mach_absolute_time();
#endif

    /* ***** Risolve il sistema lineare ****** */
    dgssv(&options, &A, permCol, permRow, &L, &U, &B, &stat, &info);	

#ifdef WIN32
	/* Acquisce un timestamp ad alta risoluzione (WIN32 API)	  */
	QueryPerformanceCounter(&EndingTime);
	/* Calcola il numero di microsecondi impiegati per il calcolo */
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	/* Converte il millisecondi								      */
	solutionHdlr->solutionTime = (unsigned long) (ElapsedMicroseconds.QuadPart);
#elif __APPLE__
	/* Acquisce un timestamp in mach_absolute_time */
	EndingTime = mach_absolute_time();
	/* Calcola la differenza e la converte in millisecondi */
	Elapsed = EndingTime - StartingTime;
	(void) mach_timebase_info(&sTimebaseInfo);
	ElapsedNano = Elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
	ElapsedNano = ElapsedNano / 1000000;
	solutionHdlr->solutionTime =  ElapsedNano;
#endif

#ifdef LIBSLU_FILE_TEST
	//Scrive vettore soluzione in file di test
	libslu_print_vector_file("/Users/7iacura/Documents/mcs/testSolution.txt", b, m);
	printf("testSolution ");
#endif

    /* Copia risultati in struttura LIBSLU_SOLUTION */
    solutionHdlr->rows = m;
    solutionHdlr->cols = n;
    solutionHdlr->nonZero = nonZero;
    solutionHdlr->solution = b;
	libslu_relative_error(b, xe, m, &(solutionHdlr->relError));
   
    ret = LIBSLU_NO_ERR;

    error_free:

    	if (permRow != NULL)
    	{
    		free(permRow);
    	}
    	if (permCol != NULL)
    	{
    		free(permCol);
    	}
    	if (xe != NULL)
    	{
    		free(xe);
    	}

    	Destroy_CompCol_Matrix(&A);
    	Destroy_SuperMatrix_Store(&B);
    	Destroy_SuperNode_Matrix(&L);
    	Destroy_CompCol_Matrix(&U);
    	StatFree(&stat);

    	return ret;
    	
}

/* Moltiplica una matrice per un vettore. Ritorna un puntatore al risultato, per il quale alloca la memoria necessaria */
double* libslu_matrix_vector(int m, int n, int ncc, int icc[], int ccc[], double acc[], double x[])
{
	double *b;
	int i;
	int j;
	int k;

	b = (double *)malloc(m * sizeof(double));

	for (i = 0; i < m; i++)
	{
		b[i] = 0.0;
	}

	for (j = 0; j < n; j++)
	{
		for (k = ccc[j]; k < ccc[j + 1]; k++)
		{
			i = icc[k];
			b[i] = b[i] + acc[k] * x[j];
		}
	}
	return b;
}

LIBSLU_RET_CODE  libslu_print_solution(LIBSLU_SOLUTION* solutionHdlr, BOOL debug)
{
	/* Verifica validità parametri */
	if (solutionHdlr == NULL)
	{
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "NULL parameter - solutionHdlr");
		}
		return LIBSLU_ERR_PARAM;
	}

	printf("MATRIX: %s\r\n", solutionHdlr->matrixFile);
	printf("rows: %u\r\n", solutionHdlr->rows);
	printf("columns: %u\r\n", solutionHdlr->cols);
	printf("Non zero elements: %u\r\n", solutionHdlr->nonZero);
	printf("Solution time: %u ms\r\n", solutionHdlr->solutionTime);
	printf("Relative error: %.30lf\r\n", solutionHdlr->relError);
	printf("Memory usage: %u\r\n", solutionHdlr->memoryUsage);
	printf("\r\n");

	return LIBSLU_NO_ERR;
}

LIBSLU_RET_CODE  libslu_free(LIBSLU_SOLUTION* solutionHdlr, BOOL debug)
{
	/* Verifica validità parametri */
	if (solutionHdlr == NULL)
	{
		if (debug == TRUE)
		{
			printf("LIBSLU ERROR : %d (%s)\r\n", LIBSLU_ERR_PARAM, "NULL parameter - solutionHdlr");
		}
		return LIBSLU_ERR_PARAM;
	}

	if (solutionHdlr->matrixFile != NULL)
	{
		free(solutionHdlr->matrixFile);
	}

	return LIBSLU_NO_ERR;

}

LIBSLU_RET_CODE libslu_read_mm_file(FILE* fp, int* n, int* m, int* nonZero, double** nzVal, int** rowind, int** colptr)
{
#define MAX_LINE_LEN	512

	char lineBuf[MAX_LINE_LEN];				/* Buffer in cui viene copiata la riga corrente del file */
	int curCol = 0;							/* Indice colonna corrente durante scansione file        */
	int curItem = 0;						/* Indice elemento corrente					             */
	double* nzVal_int = NULL;
	int* rowind_int = NULL;
	int* colptr_int = NULL;
	int ret;

	memset(lineBuf, '\0', sizeof(lineBuf));

	/* Scarta i commenti iniziali (righe che iniziano con '%') */
	while (fgets(lineBuf, MAX_LINE_LEN, fp) != NULL)
	{
		if (lineBuf[0] == '%')
		{
			continue;
		}
		else
		{
			break;
		}
	}

	/* Legge prima riga */
	if (sscanf(lineBuf, "%d %d %d", n, m, nonZero) != 3)
	{
		/* Errore intestazione file */
		return LIBSLU_ERR_MM_HEADER;
	}

	/* Alloca i buffer per la rappresentazione CC */
	nzVal_int = (double*) malloc((*nonZero) * sizeof(double));
	rowind_int = (int*) malloc((*nonZero) * sizeof(int));
	colptr_int = (int*) malloc(((*n) + 1) * sizeof(int));
	if (nzVal_int == NULL || rowind_int == NULL || colptr_int == NULL)
	{
		ret = LIBSLU_ERR_MALLOC;
		goto error_free;
	}

	/* Legge le righe della matrice */
	while (fgets(lineBuf, MAX_LINE_LEN, fp) != NULL)
	{
		int col;
		int row;
		double value;

		if (sscanf(lineBuf, "%d %d %lf", &row, &col, &value) != 3)
		{
			/* Errore */
			ret = LIBSLU_ERR_MM_BODY;
			goto error_free;
		}

		nzVal_int[curItem] = value;
		rowind_int[curItem] = row - 1;
		
		if (col != curCol)
		{
			colptr_int[col - 1] = curItem;
			curCol = col;
		}
		curItem++;
	}
	colptr_int[(*n)] = curItem;

	*nzVal = nzVal_int;
	*rowind = rowind_int;
	*colptr = colptr_int;

	return LIBSLU_NO_ERR;

error_free:

	if (nzVal_int != NULL)
	{
		free(nzVal_int);
	}
	if (rowind_int != NULL)
	{
		free(rowind_int);
	}
	if (colptr_int != NULL)
	{
		free(colptr_int);
	}

	return ret;
}

LIBSLU_RET_CODE libslu_relative_error(double x[], double xe[], int len, double* relError)
{
	int i;
	double norm1 = 0;
	double norm2 = 0;

	for (i = 0; i < len; i++)
	{
		norm1 = norm1 + pow((x[i] - xe[i]), 2);
	}
	norm1 = sqrt(norm1);

	for (i = 0; i < len; i++)
	{
		norm2 = norm2 + pow(xe[i], 2);
	}
	norm2 = sqrt(norm2);

	*relError = norm1 / norm2; 
}

void libslu_print_vector_file(char* filePath, double* solution, int solutionLen)
{
	int i;
	FILE *fp;

	fp = fopen(filePath, "wb");
	if (fp == NULL)
	{
		return; 
	}

	for(i = 0; i < solutionLen; i++)
	{
		fprintf(fp, "%.30lf\r\n", solution[i]);
	}
	fclose(fp);
}
