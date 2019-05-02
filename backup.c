//Basic dependences
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//File dependences
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//For copying files
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//Thread dependences
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>

//My dependencies
#include "backup.h"

//Steps:
//Get list of FILEs in directory we want to backup
//After setting up a backup directory, create new files based on the list of FILEs we have.
//Then, iterate through the files & copy the contents
//!!!!!!!!!!MAKE sure we iterate through subdirectories properly too. Probally using recursion.

//To compile
//gcc -pthread -g -o Backup backup.c

int main()
{
	FileNode *pHead = NULL;

	//Ask the user to input the file he wants to backup
	char backupDir[256];
	receiveDirectory(backupDir);

	//Initalize our linkedList with our backup files
	pHead = getFileContents(backupDir, pHead);
	//printFileContents(pHead);

	//TODO: ask the user the time interval of backup's they want
	char timeInterval[256];
	receiveBackupInterval(timeInterval);

	//Run thread here
	//define arguments
	BackupArgs *args = (BackupArgs *)malloc(sizeof(BackupArgs));
	strcpy(args->origDir, backupDir);
	args->timeInterval = convertTimeToSeconds(timeInterval);
	//args->pHead = pHead;

	pthread_t tid;
	pthread_create(&tid, NULL, backupThread, (void *)args);
	pthread_join(tid, NULL);
	return 0;
}

FileNode *pushToList(FileNode *pHead, File file)
{

	FileNode *pNew = (FileNode *)malloc(sizeof(FileNode));
	pNew->file = file;
	pNew->pNext = pHead;

	return pNew;
}

File initFile(char name[256], int type)
{
	File file;
	strcpy(file.name, name);
	file.type = type;

	return file;
}

FileNode *getFileContents(char directory[256], FileNode *pHead)
{
	int index = 0;

	DIR *d;
	struct dirent *dir;
	d = opendir(directory);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".backup") != 0) //Ignore . & .. directory & .backup directory
			{
				//Create file to add to linkedlist
				File file = initFile(dir->d_name, dir->d_type);

				if (file.type == fDIRECTORY)
				{
					char dirPath[256];
					strcpy(dirPath, directory); //Initalize the current folder directory were in

					char fileEndpath[256]; //Addeds / to the folder, e.g. Pickle/
					strcpy(fileEndpath, file.name);
					strcat(fileEndpath, "/");

					strcat(dirPath, fileEndpath);			 //Combines our current path to our suboflder. e.g ./Main & Pickle = ./Main/Pickle/
					pHead = getFileContents(dirPath, pHead); //Go inside the subfolder & obtain files in there.
				}

				strcpy(file.path, directory);	//Finish initalizing file by specifying its path
				pHead = pushToList(pHead, file); //Push file to linkedlist.
				index++;
			}
		}
		closedir(d);
	}
	else
	{
		printf("Error: File not found\n");
	}

	return pHead;
}

//Create directories /w empty files in a new backup destination
void createBackupFile(char *origDir, char *backupName, FileNode *pHead)
{
	char backupDir[256];

	//Create the main ./backup/ dir if it doesnt exist already.
	strcpy(backupDir, origDir);
	strcat(backupDir, ".backup/");
	createDirectory(backupDir);

	//Create the main backup file with out timestamp.
	strcpy(backupDir, strcat(backupDir, backupName));
	createDirectory(backupDir);

	FileNode *p;
	for (p = pHead; p != NULL; p = p->pNext)
	{
		if (p->file.type == fDIRECTORY)
		{
			char newDir[256];
			//TODO: repalce "./" with our true base directory provided. e.g. ./BackupFile/
			strcpy(newDir, replace_str(p->file.path, origDir, backupDir));
			//Then append the filename at the end of the path.
			strcat(newDir, p->file.name);
			createDirectory(newDir);
		}
	}

	//Then add our files
	for (p = pHead; p != NULL; p = p->pNext)
		if (p->file.type == fFILE)
		{
			char newDir[256];
			strcpy(newDir, replace_str(p->file.path, origDir, backupDir));
			//Append the newDir & filename together
			strcat(newDir, p->file.name);

			//Create new file at the linkedlist destination
			FILE *newFile = fopen(newDir, "wb");

			//Read the old file name (+ adds on the filename to the path)
			char combinedFilePath[256];
			strcpy(combinedFilePath, p->file.path);
			strcat(combinedFilePath, p->file.name);
			FILE *oldFile = fopen(combinedFilePath, "rb"); //Open the old file
			int i;
			for (i = getc(oldFile); i != EOF; i = getc(oldFile))
			{
				putc(i, newFile);
			}
			fclose(newFile);
			fclose(oldFile);

			//printf("New Dir: %s | Old Dir: %s\n", newDir, strcat(p->file.path, p->file.name));
			//Copy the contents of the old file to the new one we just created.
		}
}

void printFileContents(FileNode *pHead)
{

	FileNode *p;
	for (p = pHead; p != NULL; p = p->pNext)
	{
		if (p->file.type == fDIRECTORY)
		{
			printf("Folder: %s | Path: %s\n", p->file.name, p->file.path);
		}
		else
			printf("File: %s | Path: %s\n", p->file.name, p->file.path);
	}
}

void createDirectory(char *name)
{
#if defined(_WIN32)
	_mkdir(name);
#else
	mkdir(name, 0700);
#endif
}

//!!!
//TODO: remove these two methods and have a universal input method which i can handle input errors
//!!!

void receiveDirectory(char *dir)
{
	printf("Enter a directory to backup: \n");
	scanf("%s", dir);

	strcpy(dir, validateDirectory(dir)); //Couldn't i use a double pointer with this? How?
}

void receiveBackupInterval(char *time)
{
	printf("Enter a backup interval in proper format: e.g. (1h30m10s):\n");
	scanf("%s", time);
}

char *validateDirectory(char *dir)
{
	char validDir[256];
	//Check if there is a / at the end of the file
	char lastChar = dir[strlen(dir) - 1];
	if (lastChar != '/')
	{ //If there is no / at the end of the directory, add it for our program to work.
		char c = '/';
		strncat(dir, &c, 1);
	}
	return dir;
}

//String manipulation methods

//Not my original code.
char *replace_str(char *str, char *orig, char *rep)
{
	static char buffer[256];
	char *p;

	if (!(p = strstr(str, orig))) // Is 'orig' even in 'str'?
		return str;

	strncpy(buffer, str, p - str); // Copy characters from 'str' start to 'orig' st$
	buffer[p - str] = '\0';

	sprintf(buffer + (p - str), "%s%s", rep, p + strlen(orig));

	return buffer;
}

char *replaceCharacter(char *str, char orig, char rep)
{
	int i;
	for (i = 0; i < strlen(str); i++)
		if (str[i] == orig)
			str[i] == rep;

	return str;
}

int convertTimeToSeconds(char *str)
{
	//Example input: 2h30m10s

	int time = 0;

	//number variables
	int index = 0;
	char currNumber[256];

	int i;
	for (i = 0; i < strlen(str); i++)
	{
		//
		if (isdigit(str[i]))
		{
			currNumber[index] = str[i];
			index++;
		}
		else if (isalpha(str[i])) //Else if we came across a time character
		{
			int num = atoi(currNumber); //Convert to digit
			switch (str[i])
			{
			case 'h':
				time += (num * 3600);
				break;

			case 'm':
				time += (num * 60);
				break;

			case 's':
				time += num;
				break;

			default:
				printf("Inavlid time character\n");
				break;
			}
			memset(currNumber, 0, strlen(currNumber));
			index = 0;
		}
	}
	return time;
}

char *getCurrentTime()
{
	static char str[100];
	time_t now = time(0);
	strftime(str, sizeof(str), "%Y-%m-%d %H꞉%M꞉%S", localtime(&now));
	return str;
}

//Repeating thread test
void *backupThread(void *args)
{
	while (1)
	{
		FileNode *pHead = NULL;
		pHead = getFileContents(((BackupArgs *)args)->origDir, pHead);

		//Create the backupfolder name (also includes the final dir name)
		char backupName[256];
		strcpy(backupName, getCurrentTime());
		strcat(backupName, "/"); //To finish the file name

		createBackupFile(((BackupArgs *)args)->origDir, backupName, pHead);
		printf("Backup created sucessfully at %s\n", getCurrentTime());
		sleep(((BackupArgs *)args)->timeInterval);
	}
}
