#include "save_manager.h"
#include "fs.h"

#include <3ds/result.h>
#include <3ds/os.h>

#include <stdio.h>

#define SAVEDATA_MAX_SIZE 0x76000 // SAVEDATA_ORAS_SIZE

static const char* baseFolder = "/pk/";
static const char* saveFolder = "/pk/save/";
static const char* backupFolder = "/pk/backup/";

Result Save_exportSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8 savedata[SAVEDATA_MAX_SIZE];

	ret = FS_CreateDirectory((char*) baseFolder, &sdmcArchive);
	ret = FS_CreateDirectory((char*) saveFolder, &sdmcArchive);

	sprintf(path, "main");
	ret = FS_ReadFile(path, (void*) savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	if (R_FAILED(ret)) return ret;

	sprintf(path, "%smain", (char*) saveFolder);
	ret = FS_DeleteFile(path, &sdmcArchive);
	ret = FS_WriteFile(path, (void*) savedata, bytesRead, &sdmcArchive, &bytesWritten);
	if (R_FAILED(ret)) return ret;

	return ret;
}


Result Save_importSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8 savedata[SAVEDATA_MAX_SIZE];

	sprintf(path, "%smain", (char*) saveFolder);
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	if (R_FAILED(ret)) return ret;

	sprintf(path, "main");
	ret = FS_DeleteFile(path, &saveArchive);
	ret = FS_WriteFile(path, (void*) savedata, bytesRead, &saveArchive, &bytesWritten);
	if (R_FAILED(ret)) return ret;

	return ret;
}


Result Save_backupSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8 savedata[SAVEDATA_MAX_SIZE];

	ret = FS_CreateDirectory((char*) baseFolder, &sdmcArchive);
	ret = FS_CreateDirectory((char*) backupFolder, &sdmcArchive);

	sprintf(path, "main");
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	if (R_FAILED(ret)) return ret;

	sprintf(path, "%smain_%lli", backupFolder, osGetTime() / 1000 - 2208988800L);
	ret = FS_WriteFile(path, (void*) savedata, bytesRead, &saveArchive, &bytesWritten);

	return ret;
}
