#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct fheader
{
    char name[255];
    size_t size;
};


int main(int argc, char *argv[])
{
    char *error = "You entered an invalid sludge command -- exiting!";
    char *list = "-l";
    char *extract = "-e";
    char *create = "-c";
    char *add = "-a";
    char *remove = "-r";
    char *sludge;
    char *func = argv[1];
    
    
    if (strncmp(create, func, 2) == 0)
    {
        printf("You want to create!\n");
        FILE *_fwrite;
        FILE *_fread;
        char *fname;
        char string[1000];
        int file = 0;
        struct stat sb;

        int i;
        _fwrite = fopen(argv[2], "w+");
        for (i=3;i<argc;i++)
        {
            stat(argv[i], &sb);
            struct fheader *_fhead = (struct fheader *)malloc(sizeof(struct fheader));
            strcpy(_fhead->name, argv[i]);
            _fhead->size = sb.st_size;
            fwrite(_fhead, sizeof(struct fheader), 1, _fwrite);
            _fread = fopen(argv[i], "r");
            fputs("\n", _fwrite);
            if (fgets(string,1000,_fread) != NULL)
                fputs(string, _fwrite);
            fclose(_fread);
        }
        fclose(_fwrite);
        return 0;
    } else if (strncmp(remove, func, 2) == 0) {
        printf("You want to remove!\n");
        return 0;
    } else if (strncmp(add, func, 2) == 0) {
        printf("You want to add!\n");
        return 0;
    } else if (strncmp(extract, func, 2) == 0) {
        if (argc == 3)
        {
            sludge = argv[2];
            printf("Extracting all from %s\n", sludge); 
       
            FILE *_fread;
            FILE *_fwrite;
            struct fheader *_fhead = (struct fheader *)malloc(sizeof(struct fheader));
            int pos = 0;
            _fread = fopen(argv[2], "r");
            while (1)
            {
                fseek(_fread, pos, SEEK_CUR);
                if (fread(_fhead, sizeof(struct fheader), 1, _fread) == 0)
                    break;
                _fwrite = fopen(_fhead->name, "w+");
                fseek(_fread, pos + sizeof(struct fheader), SEEK_CUR);
                fwrite(_fread, sizeof(char), _fhead->size, _fwrite);
                pos = _fhead->size;
                close(_fwrite);
            }
        } else {
            printf("You want to extract some files!\n");
        }
        return 0;

    } else if (strncmp(list, func, 2) == 0) {
        FILE *_fread;
        struct fheader *_fhead = (struct fheader *)malloc(sizeof(struct fheader));
        int pos = 0;
        _fread = fopen(argv[2], "r");
        while (1)
        {
            fseek(_fread, pos, SEEK_CUR);
            if (fread(_fhead, sizeof(struct fheader), 1, _fread) == 0)
                break;
            printf("%s\n", _fhead->name);
            pos = _fhead->size;
        }
        return 0;
    } else {
        printf("%s", argv[2]);
        printf("%s\n", error);
        exit(1);
    }       
}


