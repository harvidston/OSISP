#include <stdio.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <malloc.h>

/*
*Написать программу поиска одинаковых по содержимому файлов в двух каталогов, например, Dir1 и Dir2. 
*Пользователь задаёт имена Dir1 и Dir2. В результате работы программы файлы, имеющиеся в Dir1, сравниваются
*с файлами в Dir2 по их содержимому. Процедуры сравнения должны запускаться в отдельном процессе для каждой 
*пары сравниваемых файлов. Каждый процесс выводит на экран свой pid, имя файла, общее число просмотренных 
*байт и результаты сравнения. Число одновременно работающих процессов не должно превышать N (вводится пользователем).
*Скопировать несколько файлов из каталога /etc в свой домашний каталог. Проверить работу программы для каталога /etc 
*и домашнего каталога.
*/

static int amount = 0;

void printErr(char *fileName, char *path) 
{
    char err[PATH_MAX + strlen(fileName) + 2];
    sprintf(err, "%s: %s", fileName, path);
    perror(err);
}

void fillDirFiles(char *dirname, char **dirFiles, char *fileName) 
{
    errno = 0;
    DIR *dir = opendir(dirname);
    if (dir == NULL) 
    {
        amount++;   
        printErr(fileName, dirname);
        return;
    }

    struct dirent *d;
    while ((d = readdir(dir)) != NULL) 
    {
        if (strcmp(".", d->d_name) != 0 && strcmp("..", d->d_name) != 0) 
        {
            char fullPath[PATH_MAX];
            strcpy(fullPath, dirname);
            fullPath[strlen(fullPath) + 1] = '\0';
            fullPath[strlen(fullPath)] = '/';
            strcat(fullPath, d->d_name);
           
            struct stat s;
            stat(fullPath, &s);

            if (S_ISDIR(s.st_mode)) 
            {
                fillDirFiles(fullPath, dirFiles, fileName);
            } 
            else
            {
                if (S_ISREG(s.st_mode)) 
                {
                    errno = 0;
                    FILE *f = fopen(fullPath, "r");
                    if (f == NULL) 
                    {
                
                        printErr(fileName, fullPath);
                    } 
                    else 
                    {
                        dirFiles[amount] = (char*)malloc(sizeof(char) * PATH_MAX);
                        strcpy(dirFiles[amount], fullPath);
                        amount++;

                        errno = 0;
                        if (fclose(f) == EOF) 
                        {
                            printErr(fileName, fullPath);
                        }
                    }
                }
            } 
            
        }
    }

    errno = 0;
    if (closedir(dir) == -1) 
    {
        printErr(fileName, dirname);
    }
}

int isValidDirectory(char *dirName, char *file)
{
    errno = 0;
    DIR *dir = opendir(dirName);
    if (dir == NULL)
    {
        printErr(file, dirName);
        return 0;
    }

    errno = 0;
    if (closedir(dir) == -1) 
    {
        printErr(file, dirName);
        return 0;
    }

    return 1;
}

int isNumber(char *number, char *file)
{
	for(int i = 0; i < strlen(number); i++)
	{
		if(number[i] < '0' || number[i] > '9')
		{
            fprintf(stderr, "%s: 3rd argument: not a number or number < 0\n", file);
			return 0;
		}
	}

	return 1;
}

int isValidInput(int argc, char *argv[])
{
    int isValid = 1;

    if (argc < 4)  
    {
        fprintf(stderr, "%s: Too few arguments\n", basename(argv[0]));
        return 0;
    }

    if (!isValidDirectory(argv[1], basename(argv[0])))
    {
        isValid = 0;
    }

    if (!isValidDirectory(argv[2], basename(argv[0])))
    {
        isValid = 0;
    }

    if (!isNumber(argv[3], basename(argv[0])))
    {
        isValid = 0;
    }

    return isValid;
}

void readFile(FILE *file, char *path, char *name, char *buffer)
{
    unsigned long bytesRead = 0;
    while(!feof(file))
    {
        bytesRead = fread(buffer, 1, PATH_MAX, file);
        if (bytesRead != PATH_MAX) 
        {
            if (ferror(file)) 
            {
                printErr(name, path);
                break;
            }
        }
    }
}

void checkForMatch(char *firstFilePath, char *secondFilePath, char *name)
{
    errno = 0;
    if(firstFilePath == NULL)
    {
        return;
    }

    FILE *firstFile = fopen(firstFilePath, "r");
    if (firstFile == NULL)
    {
        printErr(name, firstFilePath);
        return;
    }

    errno = 0;
    if(secondFilePath == NULL)
    {
        return;
    }

    FILE *secondFile = fopen(secondFilePath, "r");
    if (secondFile == NULL)
    {
        printErr(name, secondFilePath);
        return;
    }

    char firstBuff[PATH_MAX]; 
    char secondBuff[PATH_MAX];
    readFile(firstFile, firstFilePath, name, firstBuff);
    readFile(secondFile, secondFilePath, name, secondBuff);
    fclose(firstFile);
    fclose(secondFile);

    unsigned long pos = 0;
    while ((pos < strlen(firstBuff) && pos < strlen(secondBuff)) && (firstBuff[pos] == secondBuff[pos]))
    {
        pos++;
    }

    if ((strlen(firstBuff) == strlen(secondBuff)) && strlen(firstBuff) == pos)
    {
        fprintf(stdout, "%d: %s %s %ld =\n", getpid(), firstFilePath, secondFilePath, pos + 1);
    }
    else
    {
        fprintf(stdout, "%d: %s %s %ld !=\n", getpid(), firstFilePath, secondFilePath, pos + 1);
    }
}

void splitIntoThreads(char **firstFiles, int firstAmount, char **secondFiles, int secondAmount, int max, char *name)
{
    int currAmount = 0;
    int secondCounter;
    pid_t pid = -1;

    while(firstAmount > 0)
    {
        secondCounter = 0;
        while(secondCounter != secondAmount)
        {
            if (currAmount >= max)
            {
                wait(0);
                currAmount--;
            }

            do
            {
                pid = fork();
                if (pid == -1)
                {
                    fprintf(stderr, "%s: %s\n", name, strerror(errno));
                    errno = 0;
                }
                else
                {
                    if (pid == 0)
                    {
                        checkForMatch(firstFiles[firstAmount - 1], secondFiles[secondCounter], name);
                        exit(0);
                    }
                    else
                    {
                        if (pid > 0)
                        {
                            currAmount++;
                            secondCounter++;
                        }
                    }
                }
            } while (pid == -1);
        }

        firstAmount--;
    }
}

int main(int argc, char *argv[])
{
    if (isValidInput(argc, argv) == 0)
    {   
        return -1;
    }

    char *firstDir = argv[1];
    char *secondDir = argv[2];
    int threadsMaxCount = atoi(argv[3]);

    char **firstDirFiles = (char**)malloc(4096 * 4096 * sizeof(char*));
    fillDirFiles(firstDir, firstDirFiles, basename(argv[0]));
    int firstDirAmount = amount;
    
    amount = 0;
    char **secondDirFiles = (char**)malloc(4096 * 4096 * sizeof(char*));
    fillDirFiles(secondDir, secondDirFiles, basename(argv[0]));
    int secondDirAmount = amount;
    
    threadsMaxCount--;
    splitIntoThreads(firstDirFiles, firstDirAmount, secondDirFiles, secondDirAmount, threadsMaxCount, basename(argv[0]));

    while (wait(NULL) != -1) {}
    
    return 0;
}