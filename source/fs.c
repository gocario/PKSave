#include "fs.h"

#include <3ds/services/fs.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/svc.h>

#define DEBUG_FS

#ifdef DEBUG_FS
#include <stdio.h>
#endif

FS_Archive sdmcArchive;
FS_Archive saveArchive;

Result FS_ReadFile(char* path, void* dst, FS_Archive* archive, u64 maxSize, u32* bytesRead)
{
	if (!path || !dst || !archive) return -1;

	Result ret;
	u64 size;
	Handle fileHandle;

#ifdef DEBUG_FS
	printf("FS_ReadFile:\n");
#endif

	ret = FSUSER_OpenFile(&fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, FS_ATTRIBUTE_DIRECTORY);
#ifdef DEBUG_FS
	printf(" > FSUSER_OpenFile: %lx\n", ret);
#endif

	if (R_SUCCEEDED(ret))
	{
		ret = FSFILE_GetSize(fileHandle, &size);
#ifdef DEBUG_FS
		printf(" > FSFILE_GetSize: %lx\n", ret);
#endif
		if (R_FAILED(ret) || size > maxSize) ret = -2;
	}

	if (R_SUCCEEDED(ret))
	{
		ret = FSFILE_Read(fileHandle, bytesRead, 0, dst, size);
#ifdef DEBUG_FS
		printf(" > FSFILE_Read: %lx\n", ret);
#endif
		if (R_FAILED(ret) || *bytesRead < size) ret = -3;
	}

	ret = FSFILE_Close(fileHandle);
#ifdef DEBUG_FS
	printf(" > FSFILE_Close: %lx\n", ret);
#endif

	return ret;
}


Result FS_WriteFile(char* path, void* src, u64 size, FS_Archive* archive, u32* bytesWritten)
{
	if (!path || !src || !archive) return -1;

	Result ret;
	Handle fileHandle;

#ifdef DEBUG_FS
	printf("FS_WriteFile:\n");
#endif

	ret = FSUSER_OpenFile(&fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_WRITE | FS_OPEN_CREATE, FS_ATTRIBUTE_DIRECTORY);
#ifdef DEBUG_FS
	printf(" > FSUSER_OpenFile: %lx\n", ret);
#endif

	if (R_SUCCEEDED(ret))
	{
		ret = FSFILE_Write(fileHandle, bytesWritten, 0L, src, size, FS_WRITE_FLUSH);
#ifdef DEBUG_FS
		printf(" > FSFILE_Write: %lx\n", ret);
#endif
		if (R_FAILED(ret)) return ret;
	}

	ret = FSFILE_Close(fileHandle);
#ifdef DEBUG_FS
	printf(" > FSFILE_Close: %lx\n", ret);
#endif

	return ret;
}


Result FS_DeleteFile(char* path, FS_Archive* archive)
{
	if (!path || !archive) return -1;

	Result ret;

#ifdef DEBUG_FS
	printf("FS_DeleteFile:\n");
#endif

	ret = FSUSER_DeleteFile(*archive, fsMakePath(PATH_ASCII, path));
#ifdef DEBUG_FS
	printf(" > FSUSER_DeleteFile: %lx\n", ret);
#endif

	return ret;
}


Result FS_CreateDirectory(char* path, FS_Archive* archive)
{
	if (!path || !archive) return -1;

	Result ret;

#ifdef DEBUG_FS
	printf("FS_CreateDirectory:\n");
#endif

	ret = FSUSER_CreateDirectory(*archive, fsMakePath(PATH_ASCII, path), FS_ATTRIBUTE_DIRECTORY);
#ifdef DEBUG_FS
	printf(" > FSUSER_CreateDirectory: %lx\n", ret);
#endif

	return ret;
}


Result FS_FilesysInit(void)
{
	Result ret = 0;

#ifdef DEBUG_FS
	printf("FS_filesysInit:\n");
#endif

	ret = fsInit();
#ifdef DEBUG_FS
	printf(" > fsInit: %lx\n", ret);
#endif
	if (R_FAILED(ret)) return ret;

	sdmcArchive = (FS_Archive) { ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, NULL) };

	// TODO saveArchive (FS_Archive)

	ret = FSUSER_OpenArchive(&sdmcArchive);
#ifdef DEBUG_FS
	printf(" > FSUSER_OpenArchive: %lx\n", ret);
#endif
	if (R_FAILED(ret)) return ret;

	ret = FSUSER_OpenArchive(&saveArchive);
#ifdef DEBUG_FS
	printf(" > FSUSER_OpenArchive: %lx\n", ret);
#endif
	if (R_FAILED(ret)) return ret;

	return ret;
}


Result FS_FilesysExit(void)
{
	Result ret = 0;

#ifdef DEBUG_FS
	printf("FS_filesysExit:\n");
#endif

	ret = FSUSER_CloseArchive(&sdmcArchive);
#ifdef DEBUG_FS
	printf(" > FSUSER_CloseArchive: %lx\n", ret);
#endif

	ret = FSUSER_ControlArchive(saveArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
#ifdef DEBUG_FS
	printf(" > FSUSER_ControlArchive: %lx\n", ret);
#endif

	ret = FSUSER_CloseArchive(&saveArchive);
#ifdef DEBUG_FS
	printf(" > FSUSER_CloseArchive: %lx\n", ret);
#endif

	fsExit();
#ifdef DEBUG_FS
	printf(" > fsExit\n");
#endif

	return ret;
}
