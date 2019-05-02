
typedef struct File
{
    char name[256];
    char path[256];
    int type;

} File;

typedef struct FileNode
{
    struct FileNode *pNext;
    File file;
} FileNode;

//Arguments used to pass into the new thread.
typedef struct BackupArgs
{
    char origDir[256];
    int timeInterval;
} BackupArgs;

//Constants
#define FALSE 0
#define TRUE 1

#define fDIRECTORY 4
#define fFILE 8

//TODO: organize these methods better
FileNode *
getFileContents(char directory[256], FileNode *pHead);
void createBackupFile(char *origDir, char *backupName, FileNode *pHead);
void printFileContents(FileNode *pHead);
FileNode *pushToList(FileNode *pHead, File file);
File initFile(char name[256], int type);
void createFile(char name[256], char path[256]);
void createDirectory(char *name);
void receiveDirectory(char *dir);
void receiveBackupInterval(char *time);
char *validateDirectory(char *dir);
char *replace_str(char *str, char *orig, char *rep);
char *replaceCharacter(char *str, char orig, char rep);
int convertTimeToSeconds(char *str);
char *getCurrentTime();
void *backupThread(void *args);
