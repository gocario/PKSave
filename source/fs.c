#include "fs.h"

#include <3ds/services/fs.h>
#include <3ds/result.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>
#include <3ds/svc.h>

#include <string.h>

#define DEBUG_FS
// #define DEBUG_FIX_ARCHIVE_FS

#ifdef DEBUG_FS
#include <stdio.h>
#endif

typedef enum {
	STATE_UNINITIALIZED,
	STATE_UNINITIALIZING,
	STATE_INITIALIZING,
	STATE_INITIALIZED,
} FS_State;

static Handle fsHandle;
static FS_State fsState = STATE_UNINITIALIZED;
static bool sdmcInitialized = false;
static bool saveInitialized = false;
FS_Archive sdmcArchive;
FS_Archive saveArchive;

#ifdef DEBUG_FIX_ARCHIVE_FS
/**
 * @return Whether the archive fix is working.
 */
static bool FS_FixBasicArchive(FS_Archive** archive)
{
#ifdef DEBUG_FS
	printf("FS_FixBasicArchive:\n");
#endif

	if (!saveInitialized && *archive == &saveArchive)
	{
#ifdef DEBUG_FS
		printf("   Save archive not initialized\n");
#endif
		*archive = &sdmcArchive;
	}

	if (!sdmcInitialized && *archive == &sdmcArchive)
	{
#ifdef DEBUG_FS
		printf("   Sdmc archive not initialized\n");
#endif
		*archive = NULL;
	}

	return (*archive != NULL);
}
#endif


static Result _srvGetServiceHandle(Handle* out, const char* name)
{
	Result ret;

	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x5,4,0); // 0x50100
	strcpy((char*) &cmdbuf[1], name);
	cmdbuf[3] = strlen(name);
	cmdbuf[4] = 0x0;

	ret = svcSendSyncRequest(*srvGetSessionHandle());
	if (R_FAILED(ret)) return ret;

	if (out) *out = cmdbuf[3];

	return cmdbuf[1];
}


bool FS_IsInitialized(void)
{
	return (fsState == STATE_INITIALIZED);
	// return (sdmcInitialize && saveInitialized);
}


bool FS_IsArchiveInitialized(FS_Archive* archive)
{
	return (archive->id == ARCHIVE_SDMC && sdmcInitialized)
		|| (archive->id == ARCHIVE_SAVEDATA && saveInitialized);
}


Result FS_ReadFile(char* path, void* dst, FS_Archive* archive, u64 maxSize, u32* bytesRead)
{
	if (!path || !dst || !archive) return -1;
	
#ifdef DEBUG_FIX_ARCHIVE_FS
	if (!FS_FixBasicArchive(&archive)) return -1;
#endif

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
	if (R_FAILED(ret)) return ret;

	ret = FSFILE_GetSize(fileHandle, &size);
#ifdef DEBUG_FS
	printf(" > FSFILE_GetSize: %lx\n", ret);
#endif
	if (R_FAILED(ret) || size > maxSize) ret = -2;

	if (R_SUCCEEDED(ret))
	{
		ret = FSFILE_Read(fileHandle, bytesRead, 0x0, dst, size);
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

#ifdef DEBUG_FIX_ARCHIVE_FS
	if (!FS_FixBasicArchive(&archive)) return -1;
#endif

	Result ret;
	Handle fileHandle;

#ifdef DEBUG_FS
	printf("FS_WriteFile:\n");
#endif

	ret = FSUSER_OpenFile(&fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_WRITE | FS_OPEN_CREATE, FS_ATTRIBUTE_DIRECTORY);
#ifdef DEBUG_FS
	printf(" > FSUSER_OpenFile: %lx\n", ret);
#endif
	if (R_FAILED(ret)) return ret;

	if (R_SUCCEEDED(ret))
	{
		ret = FSFILE_Write(fileHandle, bytesWritten, 0L, src, size, FS_WRITE_FLUSH);
#ifdef DEBUG_FS
		printf(" > FSFILE_Write: %lx\n", ret);
#endif
		if (R_FAILED(ret) || *bytesWritten != size) ret = -2;
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
	
#ifdef DEBUG_FIX_ARCHIVE_FS
	if (!FS_FixBasicArchive(&archive)) return -1;
#endif

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
	
#ifdef DEBUG_FIX_ARCHIVE_FS
	if (!FS_FixBasicArchive(&archive)) return -1;
#endif

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


Result FS_CommitArchive(FS_Archive* archive)
{
	Result ret;

#ifdef DEBUG_FS
	printf("FS_CommitArchive:\n");
#endif

		ret = FSUSER_ControlArchive(*archive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, NULL, 0, NULL, 0);
#ifdef DEBUG_FS
		printf(" > FSUSER_ControlArchive: %lx\n", ret);
#endif

		return ret;
}


Result FS_fsInit(void)
{
	Result ret = 0;
	fsState = STATE_INITIALIZING;

#ifdef DEBUG_FS
	printf("FS_fsInit:\n");
#endif

	ret = _srvGetServiceHandle(&fsHandle, "fs:USER");
#ifdef DEBUG_FS
	printf(" > _srvGetServiceHandle: %lx\n", ret);
#endif
	if (R_FAILED(ret)) return ret;

	ret = FSUSER_Initialize(fsHandle);
#ifdef DEBUG_FS
	printf(" > FSUSER_Initialize: %lx\n", ret);
#endif
	if (R_FAILED(ret)) return ret;

	fsUseSession(fsHandle, false);
#ifdef DEBUG_FS
	printf(" > fsUseSession\n");
#endif

	if (!sdmcInitialized)
	{
		sdmcArchive = (FS_Archive) { ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, NULL) };

		ret = FSUSER_OpenArchive(&sdmcArchive);
#ifdef DEBUG_FS
		printf(" > FSUSER_OpenArchive: %lx\n", ret);
#endif
		if (R_FAILED(ret)) return ret;

		sdmcInitialized = true;
	}

	if (!saveInitialized)
	{
		saveArchive = (FS_Archive) { ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, NULL) };

		ret = FSUSER_OpenArchive(&saveArchive);
#ifdef DEBUG_FS
		printf(" > FSUSER_OpenArchive: %lx\n", ret);
#endif
		if (R_FAILED(ret)) return ret;

		saveInitialized = true;
	}

	if (R_SUCCEEDED(ret))
	{
		fsState = STATE_INITIALIZED;
	}

	return ret;
}


Result FS_fsExit(void)
{
	Result ret = 0;
	fsState = STATE_UNINITIALIZING;
#ifdef DEBUG_FS
	printf("FS_fsExit:\n");
#endif

	if (sdmcInitialized)
	{
		ret = FSUSER_CloseArchive(&sdmcArchive);
#ifdef DEBUG_FS
		printf(" > FSUSER_CloseArchive: %lx\n", ret);
#endif
		sdmcInitialized = false;
	}

	if (saveInitialized)
	{
		ret = FS_CommitArchive(&saveArchive);
#ifdef DEBUG_FS
		printf(" > FS_CommitArchive: %lx\n", ret);
#endif

		ret = FSUSER_CloseArchive(&saveArchive);
#ifdef DEBUG_FS
		printf(" > FSUSER_CloseArchive: %lx\n", ret);
#endif
		saveInitialized = false;
	}

// 	fsEndUseSession();
// #ifdef DEBUG_FS
// 	printf(" > fsEndUseSession\n");
// #endif

// 	ret = svcCloseHandle(fsHandle);
// #ifdef DEBUG_FS
// 	printf(" > _srvGetServiceHandle: %lx\n", ret);
// #endif

	if (R_SUCCEEDED(ret))
	{
		fsState = STATE_UNINITIALIZED;
	}

	return ret;
}
