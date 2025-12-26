#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

struct hdr {
	uint32_t w;
	uint32_t h;
	uint8_t sync;
	uint32_t win_w;
	uint32_t win_h;
	uint8_t title[128];
};

#ifdef _WIN32
void* createSharedMem(const char* name, const uint32_t size, const uint8_t reset, void** handle) {
	uint8_t* pBuf;
	*handle = (void*)CreateFileMappingA(
		INVALID_HANDLE_VALUE,	// use paging file
		NULL,					// default security
		PAGE_READWRITE,			// read/write access
		0,						// maximum object size (high-order DWORD)
		(DWORD)size,			// maximum object size (low-order DWORD)
		name);                  // name of mapping object

	if (*handle == NULL) {
		// printf("Could not open file mapping object (%d).\n", GetLastError());
		return NULL;
	}
	pBuf = (uint8_t*)MapViewOfFileEx(*handle, FILE_MAP_ALL_ACCESS, 0, 0, size, NULL);
	if (pBuf == NULL) {
		// printf("Could not map view of file (%d).\n", GetLastError());
		CloseHandle(*handle);
		return NULL;
	}
	if (reset > 0) memset(pBuf, 0, size);
	return (void*)pBuf;
}

void destroySharedMem(void* pMem, void** handle) {
	UnmapViewOfFile(pMem);
	if (handle) CloseHandle(*handle);
}

#else
void* createSharedMem(const char* name, const uint32_t size, const uint8_t reset, void** handle) {
	*handle = NULL;
	int fd = open(FILEPATH, O_RDWR | O_CREAT | (reset > 0 ? O_TRUNC : 0 ), (mode_t)0600);
	if (fd == -1) {
		//perror("Error opening file for reading");
		return NULL;
	}
	*handle = (void*)size;
	int result = lseek(fd, size - 1, SEEK_SET);
	if (result == -1) {
		close(fd);
		//perror("Error calling lseek() to 'stretch' the file");
		return NULL;
	}
	if (reset > 0) result = write(fd, "", 1);
	if (result != 1) {
		close(fd);
		//perror("Error writing last byte of the file");
		return NULL;
	}

	void* pMem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pMem == MAP_FAILED) {
		close(fd);
		//perror("Error mmapping the file");
		return NULL;
	}
	return pMem;
}

void destroySharedMem(void* pMem, void** handle) {
	munmap(pMem, (uint32_t)(*handle));
	close(fd);
}
#endif
