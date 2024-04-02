/*

    IconSync

    IconSync is a CLI program for the Amiga designed to solve the issue of missing or
    mismatched folder icons when extracting WHDLoad archives. It streamlines the process
    of copying curated icons into corresponding folders, handling everything from general
    alphabetical icons (A.info to Z.info, including 0.info for numerically named games)
    to specific extra folder icons (like Games.info, Demos.info, AGA.info). Unique to
    IconSync is its "Blank.info" icon, used for directories without a direct match, which
    is copied and renamed appropriately.

    The program also offers an optional feature to
    delete existing game folder icons linked to .slave files, replacing them with the
    "Blank.info" icon, tailored for users who prefer Glowicons to standard WHDLoad icons
    due to color scheme mismatches. Note: If source icons have been "snapshoted", their
    positions will be duplicated, possibly leading to disorganized appearances, thus it
    is recommended to snapshot only essential icons and use the Workbench clean-up functions
    if needed. Usage involves specifying a target directory and an icon store directory,
    with an optional flag to delete existing game folder icons.

  */

#include <ctype.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <clib/utility_protos.h>
#include <proto/utility.h>

// Global variables
int replaceIcons;
int replaceGameFolderIcons;
char iconCollectionPath[256];

int IconCopyMode;
typedef struct
{
    STRPTR fileName;
} IconFile;
extern struct ExecBase *SysBase;

void checkAndDisplayMissingIcon(char *folderPath, char *folderName);
void crawlFoldersRecursive(char *parentPath);
void removeTextAfterLastSlash(char *path);
void sanitizeAmigaPath(char *str);
void crawlFolders(char *folderPath);
BOOL ContainsSlaveFile(BPTR lock);

#define _BLACK_TEXT_ "\x1b[30m"
#define _WHITE_TEXT_ "\x1b[31m"
#define _BLUE_TEXT_ "\x1b[32m"
#define _GREY_TEXT_ "\x1b[33m"
#define _RESET_TEXT_ "\x1b[0m"

/* Black text:  printf("\x1B[30m 30:\x1B[0m \n"); */
/* White text:  printf("\x1B[31m 31:\x1B[0m \n"); */
/* Blue text:   printf("\x1B[32m 32:\x1B[0m \n"); */
/* Grey text:   printf("\x1B[33m 33:\x1B[0m \n"); */

void crawlFolders(char *folderPath)
{
    sanitizeAmigaPath(folderPath);
    /* Start recursion with the initial folder path */

    IconCopyMode = 1;
    crawlFoldersRecursive(folderPath);
}

void crawlFoldersRecursive(char *parentPath)
{

    BPTR lock;
    struct FileInfoBlock *fib;
    char fullPath[256];
    BOOL slaveFileFound = FALSE;

    printf("Checking %s\n", parentPath);

    lock = Lock(parentPath, ACCESS_READ);
    if (lock == NULL)
    {

        printf("!! Failed to lock %s\n", parentPath);
        return;
    }

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if (fib == NULL)
    {
        printf("!! Failed to allocate memory for FileInfoBlock for  %s\n", parentPath);
        UnLock(lock);
        return;
    }

    // First, check for the presence of any *.slave files
    if (Examine(lock, fib))
    {
        while (ExNext(lock, fib) && !slaveFileFound)
        {
            if (fib->fib_DirEntryType < 0)
            { // Regular file
                const char *fileExt = strrchr(fib->fib_FileName, '.');
                if (fileExt && stricmp(fileExt, ".slave") == 0)
                {
                    slaveFileFound = TRUE; // .slave file found, no need to check further
                                           // printf("slave found - leaving dir %s\n",parentPath);
                }
            }
        }
    }

    // If a .slave file is found, clean up and return early
    if (slaveFileFound)
    {
        FreeDosObject(DOS_FIB, fib);
        UnLock(lock);
        return;
    }

    // If no .slave file is found, proceed with the icon checks
    // Reset the Examine to start from the beginning of the directory
    Examine(lock, fib);
    while (ExNext(lock, fib))
    {
        if (fib->fib_DirEntryType > 0)
        { // Directory
            /* Construct full path for the subdirectory */
            if (strlen(parentPath) + strlen(fib->fib_FileName) + 1 < sizeof(fullPath) - 1)
            {
                sprintf(fullPath, "%s/%s", parentPath, fib->fib_FileName);

                if (IconCopyMode != 0)
                {
                    checkAndDisplayMissingIcon(fullPath, fib->fib_FileName);
                }

                /* Recursively call for the subdirectory */
                crawlFoldersRecursive(fullPath);
            }
            else
            {
                printf("!! Path too long, skipping... %s/%s\n", parentPath, fib->fib_FileName);
            }
        }
    }

    FreeDosObject(DOS_FIB, fib);
    UnLock(lock);
}

void checkAndDisplayMissingIcon(char *folderPath, char *folderName)
{
    char iconPath[257];
    char iconPathToCopy[257];
    char targetPath[257];
    char copyCommand[515];
    int iconFound = 0;
    BPTR lock;

    

    strcpy(targetPath, folderPath);
    /* Construct icon path */
    if (strlen(folderPath) + 6 < sizeof(iconPath) - 1)
    {
        sprintf(iconPath, "%s.info", folderPath);

        /* Check if icon exists */
        lock = Lock(iconPath, ACCESS_READ);
        if (lock != NULL)
        {
            /* If replaceIcons is set and icon exists, delete the icon */
            if (replaceIcons == 1)
            {
                /* Only delete the icon if a match is found in the icon store */
                sprintf(iconPathToCopy, "%s/%s.info", iconCollectionPath, folderName);
                if (Lock(iconPathToCopy, ACCESS_READ) != NULL)
                {
                    UnLock(lock);
                    DeleteFile(iconPath);
                    printf("Deleted icon %s\n", iconPath);
                }
            }
            else if (replaceGameFolderIcons == 1)
            {
                if (ContainsSlaveFile(lock) == TRUE)
                {
                    UnLock(lock);
                    DeleteFile(iconPath);
                    printf("Deleted game folder icon %s\n", iconPath);
                }
            }
            else
            {
                UnLock(lock);
            }
        }
    }
    else
    {
        printf("!! Icons path too long, skipping... %s.info\n", "%s.info", folderPath);
    }
    /* prevent buffer overflows...*/
    if (strlen(folderPath) + strlen(folderName) + 6 > sizeof(iconPath) - 1)
    {
        printf("!! iconCollectionPath to long... %s/%s.info\n", iconCollectionPath, folderName);
    }
    else
    {
        /*  prevent buffer overflows... */
        if (strlen(iconCollectionPath) + strlen(folderName) + 6 > sizeof(iconPathToCopy) - 1)
        {
            printf("!! iconCollectionPath to long... %s/%s.default.info\n", iconCollectionPath, folderName);
        }
        else
        {
            iconFound = 0;
            sprintf(iconPathToCopy, "%s/%s.info", iconCollectionPath, folderName);
            /* if it doesnt, then does icon with the same name exist in the store? */
            if (Lock(iconPathToCopy, ACCESS_READ) != NULL)
            {

                removeTextAfterLastSlash(targetPath);
                sanitizeAmigaPath(targetPath);
                sanitizeAmigaPath(iconPathToCopy);
                sprintf(copyCommand, "COPY \"%s\" TO \"%s%s.info\"", iconPathToCopy, targetPath, folderName);
                
                SystemTagList(copyCommand, NULL);
                printf("* COPY FROM \"%s\"\n*      TO   \"%s%s.info\"\n\n", iconPathToCopy, targetPath, folderName);
                iconFound = 1;
            }

            if (iconFound == 0)
            {
                /*  prevent buffer overflows... */
                if (strlen(iconCollectionPath) + 13 > sizeof(iconPathToCopy) - 1)
                {
                    printf("!! iconCollectionPath to long... %s/%s.default.info", iconCollectionPath, folderName);
                }
                else
                {
                    sprintf(iconPathToCopy, "%s/default.info", iconCollectionPath);
                    /* if an exact match doesnt exist, then if a default icon exists then
                       copy that one and rename it */
                    if (Lock(iconPathToCopy, ACCESS_READ) != NULL)
                    {
                        sanitizeAmigaPath(targetPath);
                        sanitizeAmigaPath(iconPathToCopy);
                        sprintf(copyCommand, "COPY \"%s\" TO \"%s.info\"\n\n", iconPathToCopy, targetPath);
                        printf("Copy command: %s\n", copyCommand);
                        SystemTagList(copyCommand, NULL);
                        printf("* COPY FROM \"%s\"\n*      TO   \"%s.info\"\n\n", iconPathToCopy, targetPath);
                    }
                }
            }
        }
    }
}

BOOL ContainsSlaveFile(BPTR lock)
{
    struct FileInfoBlock *fib;
    BOOL found = FALSE;
    STRPTR fileName;
    LONG nameLen;
    /* Allocate FileInfoBlock at the start */
    fib = AllocVec(sizeof(struct FileInfoBlock), MEMF_CLEAR);
    if (!fib)
    {
        Printf("Failed to allocate memory for FileInfoBlock.\n");
        return FALSE;
    }

    if (Examine(lock, fib))
    {
        while (ExNext(lock, fib))
        {
            if (fib->fib_DirEntryType < 0)
            { /* It's a file */
                fileName = fib->fib_FileName;
                nameLen = strlen(fileName);
                if (nameLen > 6 && Stricmp(&fileName[nameLen - 6], ".slave") == 0)
                {
                    found = TRUE;
                    break;
                }
            }
        }
    }

    FreeVec(fib);
    return found;
}

void removeTextAfterLastSlash(char *path)
{
    char *lastSlash;

    /* Find the last occurrence of '/' */
    lastSlash = strrchr(path, '/');
    if (lastSlash != NULL)
    {
        /* Terminate the string after the last '/' */
        *(lastSlash + 1) = '\0';
    }
}

/*
 * Function to sanitize an Amiga file path in-place by correcting specific path issues.
 * It ensures no slashes immediately follow a colon, replaces "//" with "/", and removes consecutive colons.
 * Assumes the input buffer is large enough for the sanitized path.
 */
void sanitizeAmigaPath(char *path)
{
    ULONG len;
    char *sanitizedPath;
    int i, j;

    if (path == NULL)
    {
        return;
    }

    len = strlen(path) + 1; /* Include null terminator*/
    sanitizedPath = (char *)AllocVec(len, MEMF_ANY | MEMF_CLEAR);
    if (sanitizedPath == NULL)
    {
        /* Memory allocation failed */
        return;
    }

    i = 0;
    j = 0;
    while (path[i] != '\0')
    {
        if (path[i] == ':')
        {
            /* Copy the colon */
            sanitizedPath[j++] = path[i++];
            /* Skip all following slashes */
            while (path[i] == '/')
            {
                i++;
            }
        }
        else if (path[i] == '/' && path[i + 1] == '/')
        {
            /* Skip redundant slashes */
            i++;
        }
        else
        {
            /* Copy other characters */
            sanitizedPath[j++] = path[i++];
        }
    }

    sanitizedPath[j] = '\0'; /* Ensure the result is null-terminated */

    /* Copy back to the original path and free the allocated memory */
    strcpy(path, sanitizedPath);
    FreeVec(sanitizedPath);
}

int main(int argc, char *argv[])
{
    char input_directory_path[256];
    int i;

    replaceIcons = 0;
    replaceGameFolderIcons = 0;
    // Start crawling from the specified root folder
    if (argc < 2)
    {
        printf("\x1B[1mUsage:\x1B[0m IconSync <target_directory> <icon_store_directory> [-DeleteExistingIcons] [-DeleteGameFolderIcons]\n\n");
        return 1;
    }
    strcpy(input_directory_path, argv[1]);
    strcpy(iconCollectionPath, argv[2]);

    // Iterate over additional arguments
    for (i = 3; i < argc; i++)
    {
        if (stricmp(argv[i], "-DeleteGameFolderIcons") == 0)
        {
            replaceGameFolderIcons = 1;
        }
        if (stricmp(argv[i], "-DeleteExistingIcons") == 0)
        {
            replaceIcons = 1;
        }
    }

    sanitizeAmigaPath(iconCollectionPath);
    sanitizeAmigaPath(input_directory_path);

    crawlFolders(input_directory_path);

    return 0;
}
