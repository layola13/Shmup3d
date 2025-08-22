#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool FS_InitFilesystem(void) {
    printf("WASM Filesystem Initialized (Adapter for filesystem.h)\n");
    return true;
}

filehandle_t* FS_OpenFile( const char *filename, char* mode ) {
    char fullpath[MAX_OSPATH];
    snprintf(fullpath, sizeof(fullpath), "%s", filename);

    printf("FS_OpenFile: Attempting to open '%s'\n", fullpath);

    FILE* std_file = fopen(fullpath, mode);

    if (!std_file) {
        printf("FS_OpenFile: FAILED to open '%s'\n", fullpath);
        return NULL;
    }

    filehandle_t* fhandle = (filehandle_t*)calloc(1, sizeof(filehandle_t));
    if (!fhandle) {
        fclose(std_file);
        return NULL;
    }

    fhandle->hFile = std_file;
    // Important: Initialize memory-related fields to NULL/0
    fhandle->bLoaded = 0;
    fhandle->filedata = NULL;
    fhandle->ptrCurrent = NULL;
    
    return fhandle;
}

int FS_UploadToRAM(filehandle_t *fhandle) {
    if (!fhandle || !fhandle->hFile) {
        return 0; // Failure
    }

    // Already loaded, do nothing.
    if (fhandle->bLoaded) {
        return 1; // Success
    }

    FILE* file = (FILE*)fhandle->hFile;

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size < 0) {
        return 0; // Failure
    }

    // Allocate memory
    void* buffer = malloc(size);
    if (!buffer) {
        return 0; // Failure
    }

    // Read the whole file
    if (fread(buffer, 1, size, file) != (size_t)size) {
        free(buffer);
        return 0; // Failure
    }
    
    // Populate the filehandle_t struct for in-memory access
    fhandle->filesize = (W32)size;
    fhandle->filedata = buffer;
    fhandle->ptrStart = (uchar*)buffer;
    fhandle->ptrCurrent = (uchar*)buffer;
    fhandle->ptrEnd = fhandle->ptrStart + size;
    fhandle->bLoaded = 1;

    printf("FS_UploadToRAM: Successfully loaded '%ld' bytes into RAM.\n", size);

    return 1; // Success
}


void FS_CloseFile( filehandle_t *fhandle ) {
    if (!fhandle) {
        return;
    }
    
    // Free the memory buffer if it was loaded
    if (fhandle->bLoaded && fhandle->filedata) {
        free(fhandle->filedata);
    }

    // Close the actual file handle
    if (fhandle->hFile) {
        fclose((FILE*)fhandle->hFile);
    }
    
    // Free the handle wrapper itself
    free(fhandle);
}

SW32 FS_Read( void *buffer, W32 size, W32 count, filehandle_t *fhandle ) {
    if (!fhandle) return 0;

    // If file is loaded in memory, read from memory buffer
    if (fhandle->bLoaded) {
        W32 bytes_to_read = size * count;
        W32 bytes_remaining = fhandle->ptrEnd - fhandle->ptrCurrent;

        if (bytes_to_read > bytes_remaining) {
            bytes_to_read = bytes_remaining;
        }

        memcpy(buffer, fhandle->ptrCurrent, bytes_to_read);
        fhandle->ptrCurrent += bytes_to_read;
        
        return bytes_to_read / size;
    }
    
    // Otherwise, read from file stream
    if (!fhandle->hFile) return 0;
    return (SW32)fread(buffer, (size_t)size, (size_t)count, (FILE*)fhandle->hFile);
}

char* FS_Gamedir(void) {
    return "data/";
}

SW32 FS_Write( const void * buffer, W32 size, W32 count, filehandle_t * stream ) {
    // Writing is not supported in this context.
    return 0;
}

void *FS_GetLoadedFilePointer( filehandle_t *fhandle, W32 origin ) {
    // The engine expects this to return a pointer to the in-memory file data.
    if (fhandle && fhandle->bLoaded && fhandle->filedata) {
        // The 'origin' parameter seems to be intended for seeking,
        // which matches the classic C fseek/SEEK_SET behavior.
        // We will assume SEEK_SET for simplicity as the lexer starts from the beginning.
        return fhandle->filedata;
    }
    return NULL;
}

char* FS_GetExtensionAddress(char* string) {
    if (!string) return "";
    char* last_dot = strrchr(string, '.');
    if (last_dot && last_dot != string) {
        return last_dot + 1;
    }
    return "";
}

char* FS_GetFilenameOnly(char* string) {
    if (!string) return "";
    char* last_slash = strrchr(string, '/');
    if (last_slash) {
        return last_slash + 1;
    }
    return string;
}