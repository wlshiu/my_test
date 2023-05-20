#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#define CONFIG_MAX_PATH_SIZE    512

static void
_usage(char *argv)
{
    printf("usage: %s <root dir>\n", argv);
    return;
}

static int
_search_files(char *pCur_dir, char *pExt)
{
    HANDLE              hFindFile;
    char                target_path[CONFIG_MAX_PATH_SIZE] = {0};
    WIN32_FIND_DATA     find_data;

    snprintf(target_path, sizeof(target_path), "%s\\*.*", pCur_dir);

//    printf("%s\n", target_path);

    hFindFile = FindFirstFile(target_path, &find_data);
    if( INVALID_HANDLE_VALUE == hFindFile )
        return -1;

    while( FindNextFile(hFindFile, &find_data) )
    {
        if( strncmp(find_data.cFileName, ".", strlen(".")) == 0 ||
            strncmp(find_data.cFileName, "..", strlen("..")) == 0 )
            continue;

        if( find_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
        {
            char    next_dir[CONFIG_MAX_PATH_SIZE] = {0};

            snprintf(next_dir, sizeof(next_dir), "%s\\%s", pCur_dir, find_data.cFileName);
//            printf("=> next: %s\n", next_dir);
            _search_files((char*)next_dir, pExt);
            continue;
        }

        char    *pCur_ext = strrchr(find_data.cFileName, '.');

        if( !pCur_ext )
            continue;

        if( pExt == 0 || strncmp(pCur_ext, pExt, strlen(pExt)) == 0 )
            printf("Get: %s\\%s\n", pCur_dir, find_data.cFileName);
    }

    FindClose(hFindFile);

    return 0;
}

int main(int argc, char **argv)
{
    char    *pRoot_dir = 0;


    do {
        if( argc < 2 )
        {
            _usage(argv[0]);
            break;
        }

        pRoot_dir = argv[1];
        _search_files(pRoot_dir, 0);
    } while(0);

    system("pause");
    return 0;
}
