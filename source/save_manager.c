#include "save_manager.h"
#include "fs.h"

#include <3ds/result.h>
#include <3ds/os.h>

#include <stdio.h>
#include <stdlib.h>

#define SAVEDATA_MAX_SIZE 0x76000 // SAVEDATA_ORAS_SIZE

static const char* rootFolder = "/";
static const char* baseFolder = "/pk/";
static const char* saveFolder = "/pk/save/";
static const char* backupFolder = "/pk/backup/";

Result Save_exportSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE * sizeof(u8));

	ret = FS_CreateDirectory((char*) baseFolder, &sdmcArchive);
	ret = FS_CreateDirectory((char*) saveFolder, &sdmcArchive);

	sprintf(path, "%smain", rootFolder);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);

	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%smain", (char*) saveFolder);
		ret = FS_DeleteFile(path, &sdmcArchive);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
	}

	free(savedata);
	return ret;
}


Result Save_importSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE * sizeof(u8));

	sprintf(path, "%smain", (char*) saveFolder);
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	
	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%smain", rootFolder);
		ret = FS_DeleteFile(path, &saveArchive);
		ret = FS_WriteFile(path, (void*) savedata, bytesRead, &saveArchive, &bytesWritten);

		if (R_SUCCEEDED(ret))
		{
			ret = FS_CommitArchive(&saveArchive);
		}
	}

	free(savedata);
	return ret;
}


Result Save_backupSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE * sizeof(u8));

	ret = FS_CreateDirectory((char*) baseFolder, &sdmcArchive);
	ret = FS_CreateDirectory((char*) backupFolder, &sdmcArchive);

	sprintf(path, "%smain", rootFolder);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	
	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%smain_%lli", backupFolder, osGetTime()/* - 2208988800L*/);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
	}

	free(savedata);

	return ret;
}
