#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "libslu.h"
#include <dirent.h>
#include <unistd.h>


#define DEBUG 1
#define MAX_DIRPATH_LEN		200
#define MAX_FILENAME_LEN     30

int main(int argc, char* argv[])
{
	LIBSLU_SOLUTION* solution;
	int errCode = 0;
	char filePath[MAX_DIRPATH_LEN + MAX_FILENAME_LEN + 1];
	char dirPath[MAX_DIRPATH_LEN + 1];
	DIR* dir;
	struct dirent *nextFile;

	/* Valida numero di parametri in ingresso */
	if (argc != 2)
	{
		printf("\nUsage: %s <directory path>\r\n", argv[0]);
		return -1;
	}
	/* Verifica lunghezza path directory */
	if (strlen(argv[1]) > MAX_DIRPATH_LEN)
	{
		printf("\nDirectory path too long. Max allowed length is %d\r\n", MAX_DIRPATH_LEN);
	}
	/* Copia path directory e concatena "\*.mtx" */
	strcpy(dirPath, argv[1]);

	/* Cerca il primo file nella directory */
	dir = opendir(dirPath);

	if (dir == NULL)
	{
		printf("\nError in opendir\r\n");
		return -1;
	}

	printf("Start libslu_main\r\n");

	while ((nextFile = readdir(dir)) != NULL)
	{
		if (nextFile->d_type == DT_REG)
		{
			/* Ottiene il path assoluto del file */
			strcpy(filePath, dirPath);
			strcat(filePath, "/");
			strcat(filePath, nextFile->d_name);

			if (strstr(filePath, ".mtx") == NULL)
			{
				continue;
			}

			/* Risolve la matrice associata al file .mtx */
			solution = libslu_init(filePath, &errCode, DEBUG);
			if (errCode != 0)
			{
				return -1;
			}

			errCode = libslu_solvesparse(solution, DEBUG);
			if (errCode != 0)
			{
				return -1;
			}

			libslu_print_solution(solution, DEBUG);
			libslu_free(solution, DEBUG);
		}
	} 

	closedir(dir);

	return 0;

}
