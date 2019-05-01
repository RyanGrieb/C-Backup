
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

//Constants
#define FALSE 0
#define TRUE 1

#define fDIRECTORY 4
#define fFILE 8

#define BACKUP_PATH "./.backup/"

//TODO: organize these methods better
FileNode *getFileContents(char directory[256], FileNode *pHead);
void createBackupFile(char *origDir, char *backupName, FileNode *pHead);
void printFileContents(FileNode *pHead);
FileNode *pushToList(FileNode *pHead, File file);
File initFile(char name[256], int type);
void createFile(char name[256], char path[256]);
void createDirectory(char *name);
void receiveInput(char *dir);
char *validateDirectory(char *dir);
char *replace_str(char *str, char *orig, char *rep);
