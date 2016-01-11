#include "save_manager.h"
#include "pkdir.h"
#include "fs.h"

#include <3ds/os.h>
#include <3ds/result.h>
#include <3ds/services/apt.h>

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

#define TID_X  (0x0004000000055D00LL)
#define TID_Y  (0x0004000000055E00LL)
#define TID_OR (0x000400000011C400LL)
#define TID_AS (0x000400000011C500LL)

static union { 
	u64 id;
	struct {
		u64 platform : 16;
		u64 contentCategory : 16;
		u64 uniqueId : 24;
		u64 variation : 8;
	};
} title;

static FS_MediaType mediaType;

Result Save_getTitleId()
{
	aptOpenSession();
	Result ret = APT_GetProgramID(&title.id);
	if (R_FAILED(ret)) title.id = 0;
	aptCloseSession();
	return ret;
}


Result Save_getMediaType()
{
	Result ret = FSUSER_GetMediaType(&mediaType);
	if (R_FAILED(ret)) mediaType = 3;
	return ret;
}


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

	if (R_SUCCEEDED(ret) && bytesRead == Save_titleIdToSize(title.id))
	{
		ret = Save_removeSecureValue();

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


u32 Save_svOffsetSize(u32 size)
{
	switch (size)
	{
		case SAVEDATA_XY_SIZE: return SV_XY_OFFSET;
		case SAVEDATA_ORAS_SIZE: return SV_ORAS_OFFSET;
		default: return 0;
	}
}


Result Save_fixSecureValue(u8* savedata, u8* svSavedata, u32 bytesRead, u32 svBytesRead)
{
	if (!savedata || !svSavedata) return -1;
	if (bytesRead != svBytesRead || bytesRead < SAVEDATA_MIN_SIZE) return -2;

	u32 offset = Save_svOffsetSize(bytesRead);

	memcpy((void*) savedata + offset, (void*) svSavedata + offset, SV_SIZE);

	return 0;
}


u32 Save_titleIdToSize(u64 titleId)
{
	switch (titleId)
	{
		case TID_X:
		case TID_Y:
			return SAVEDATA_XY_SIZE;
		case TID_OR:
		case TID_AS:
			return SAVEDATA_ORAS_SIZE;
		default:
			return 0;
	}
}


Result Save_removeSecureValue()
{
	// if (mediaType != MEDIATYPE_SD) // ASK Which one?
	if (mediaType == MEDIATYPE_GAME_CARD) return -1;

	Result ret;
	u64 in = ((u64) SECUREVALUE_SLOT_SD << 32) | (title.uniqueId << 8) | title.variation;
	u8 out;

	ret = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &in, 8, &out, 1);
	if (R_FAILED(ret)) return ret;

	return out;
}
