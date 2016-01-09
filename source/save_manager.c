#include "save_manager.h"
#include "pkdir.h"
#include "fs.h"

#include <3ds/result.h>
#include <3ds/os.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVEDATA_XY_SIZE   (0x65600)
#define SAVEDATA_ORAS_SIZE (0x76000)
#define SAVEDATA_MIN_SIZE  (SAVEDATA_XY_SIZE)
#define SAVEDATA_MAX_SIZE  (SAVEDATA_ORAS_SIZE)

#define SV_SIZE        (0x8)
#define SV_XY_OFFSET   (0x65400)
#define SV_ORAS_OFFSET (0x75e00)



Result Save_exportSavedata(void)
{
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

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
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);

	if (R_SUCCEEDED(ret))
	{
		u32 svBytesRead;
		u8* svSavedata = malloc(SAVEDATA_MAX_SIZE);

		sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
		ret = FS_ReadFile(path, (void*) svSavedata, &saveArchive, SAVEDATA_MAX_SIZE, &svBytesRead);

		ret = Save_fixSecureValue(savedata, svSavedata, bytesRead, svBytesRead);

		ret = FS_DeleteFile(path, &saveArchive);
		ret = FS_WriteFile(path, (void*) savedata, bytesRead, &saveArchive, &bytesWritten);

		if (R_SUCCEEDED(ret))
		{
			ret = FS_CommitArchive(&saveArchive);

			sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
			ret = FS_DeleteFile(path, &sdmcArchive);
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
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

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


u32 Save_svOffset(u32 bytesRead)
{
	if (bytesRead == SAVEDATA_XY_SIZE)
		return SV_XY_OFFSET;
	else if (bytesRead == SAVEDATA_ORAS_SIZE)
		return SV_ORAS_OFFSET;
	else return 0;
}


Result Save_fixSecureValue(u8* savedata, u8* svSavedata, u32 bytesRead, u32 svBytesRead)
{
	if (!svSavedata || !svSavedata) return -1;
	if (bytesRead != svBytesRead || bytesRead < SAVEDATA_MIN_SIZE) return -2;

	u32 offset = Save_svOffset(bytesRead);

	// ASK Return an error code if same secure values?

	memcpy((void*) savedata + offset, (void*) svSavedata + offset, SV_SIZE);

	return 0;
}
