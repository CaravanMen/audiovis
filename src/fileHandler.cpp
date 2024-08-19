#include <fileHandler.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

char* read_file(const char* fileName, int &bufSize)
{
    // Initialize
    FILE* fp;
    size_t size = 0;
    char* content;

    // Read file for file size
    fp = fopen(fileName, "rb");
    if (fp == NULL)
    {
        printf("[ERR] 'read_file' failed to open file '%s'\n", fileName);
        return 0;
    }
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp)+1;
    fclose(fp);

    // Read file for contents
    fopen(fileName, "r");
    // Allocate memory to content buffer
    content = (char*)malloc(size);
    memset(content, '\0', size);
    fread(content, 1, size-1, fp);
    fclose(fp);
    // Return shader contents
    bufSize = static_cast<int>(size);
    return content;
}