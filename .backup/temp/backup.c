//Basic dependences
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//File dependences
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//For copying files
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//My dependencies
#include "backup.h"

//Steps:
//Get list of FILEs in directory we want to backup
//After setting up a backup directory, create new files based on the list of FILEs we have.
//Then, iterate through the files & copy the contents
//!!!!!!!!!!MAKE sure we iterate through subdirectories properly too. Probally using recursion.

int main()
{
	FileNode *pHead = NULL;

	//Ask the user to input the file he wants to backup
	char backupDir[256];
	receiveInput(backupDir);

	//Initalize our linkedList with our backup files
	pHead = getFileContents(backupDir, pHead);
	printFileContents(pHead);

	//Ask the user which destination folder he wants to backup

	char backupName[256] = "./temp/"; //Could be the time of the backup, whatever.

	//TODO: ask the user the time interval of backup's they want

	createBackupFile(backupDir, backupName, pHead);

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
			if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) //Ignore . & .. directory
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

	return pHead;
}

//Create directories /w empty files in a new backup destination
void createBackupFile(char *origDir, char *backupName, FileNode *pHead)
{
	char dirOfBackup[256];
	strcpy(dirOfBackup, BACKUP_PATH); //Copy the base path to dirOfbackup
	strcat(dirOfBackup, backupName);  // Add the backup name to our base backup path
	createDirectory(BACKUP_PATH);	 //Create the base backup file
	createDirectory(dirOfBackup);	 //Create subfolder of our backup name

	FileNode *p;
	for (p = pHead; p != NULL; p = p->pNext)
	{
		if (p->file.type == fDIRECTORY)
		{
			char newDir[256];
			//TODO: repalce "./" with our true base directory provided. e.g. ./BackupFile/
			strcpy(newDir, replace_str(p->file.path, origDir, dirOfBackup));
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
			strcpy(newDir, replace_str(p->file.path, origDir, dirOfBackup));
			//Append the newDir & filename together
			strcat(newDir, p->file.name);

			//Create new file at the linkedlist destination
			FILE *newFile = fopen(newDir, "wb");							 //dst
			FILE *oldFile = fopen(strcat(p->file.path, p->file.name), "rb"); //src
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

void receiveInput(char *dir)
{
	printf("Enter a directory to backup: \n");
	scanf("%s", dir);

	strcpy(dir, validateDirectory(dir)); //Couldn't i use a double pointer with this? How?
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

char *getNameOfDirectory(char *str)
{
}