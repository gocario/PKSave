#include "save_manager.h"
#include "pkdir.h"
#include "fs.h"

#include <3ds/result.h>
#include <3ds/os.h>

#include <stdio.h>
#include <stdlib.h>

#define SAVEDATA_MAX_SIZE 0x76000 // SAVEDATA_ORAS_SIZE

Result Save_exportSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE * sizeof(u8));

	ret = FS_CreateDirectory((char*) pk_baseFolder, &sdmcArchive);
	ret = FS_CreateDirectory((char*) pk_saveFolder, &sdmcArchive);

	sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);

	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
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

	sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	
	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
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

	ret = FS_CreateDirectory((char*) pk_baseFolder, &sdmcArchive);
	ret = FS_CreateDirectory((char*) pk_backupFolder, &sdmcArchive);

	sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	
	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s_%s%lli", pk_backupFolder, pk_saveFile, osGetTime()/* - 2208988800L*/);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
	}

	free(savedata);

	return ret;
}
