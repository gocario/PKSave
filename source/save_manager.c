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

static u64 _titleId;

Result Save_getTitleId(u64* titleId)
{
	Result ret;

	aptOpenSession();

	ret = APT_GetProgramID(&_titleId);
	printf(" > APT_GetProgramID: %lx\n", ret);
	if (R_FAILED(ret)) _titleId = 0;

	aptCloseSession();

	if (titleId) *titleId = _titleId;
	printf("    > tid: 0x%016llx\n", _titleId);

	return ret;
}


bool Save_titleIdIsPokemon(u64 titleId)
{
	switch (titleId)
	{
		case TID_X:
		case TID_Y:
		case TID_OR:
		case TID_AS:
			return true;
		default:
			return false;
	}
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


Result Save_exportSavedata(void)
{
	printf("Save_exportSavedata\n");
	
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	ret = FS_CreateDirectory((char*) pk_baseFolder, &sdmcArchive);
	printf(" > FS_CreateDirectory: %lx\n", ret);
	ret = FS_CreateDirectory((char*) pk_saveFolder, &sdmcArchive);
	printf(" > FS_CreateDirectory: %lx\n", ret);

	sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	printf(" > FS_ReadFile: %lx\n", ret);

	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
		ret = FS_DeleteFile(path, &sdmcArchive);
		printf(" > FS_DeleteFile: %lx\n", ret);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
		printf(" > FS_WriteFile: %lx\n", ret);
	}

	free(savedata);
	return ret;
}


Result Save_importSavedata(void)
{
	printf("Save_importSavedata\n");

	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	printf(" > FS_ReadFile: %lx\n", ret);

	// Check that is the same type of game that the export's.
	if (R_SUCCEEDED(ret) && bytesRead == Save_titleIdToSize(_titleId))
	{
		ret = Save_removeSecureValue(NULL);
		printf(" > Save_removeSecureValue: %lx\n", ret);

		sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
		ret = FS_DeleteFile(path, &saveArchive);
		printf(" > FS_DeleteFile: %lx\n", ret);
		ret = FS_WriteFile(path, (void*) savedata, bytesRead, &saveArchive, &bytesWritten);
		printf(" > FS_WriteFile: %lx\n", ret);

		if (R_SUCCEEDED(ret))
		{
			ret = FS_CommitArchive(&saveArchive);
			printf(" > FS_CommitArchive: %lx\n", ret);

			sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
			ret = FS_DeleteFile(path, &sdmcArchive);
			printf(" > FS_DeleteFile: %lx\n", ret);
		}

	}

	free(savedata);
	return ret;
}


Result Save_backupSavedata(void)
{
	printf("Save_backupSavedata\n");

	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	ret = FS_CreateDirectory((char*) pk_baseFolder, &sdmcArchive);
	printf(" > FS_CreateDirectory: %lx\n", ret);
	ret = FS_CreateDirectory((char*) pk_backupFolder, &sdmcArchive);
	printf(" > FS_CreateDirectory: %lx\n", ret);

	sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	printf(" > FS_ReadFile: %lx\n", ret);
	
	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s_%s%lli", pk_backupFolder, pk_saveFile, osGetTime()/* - 2208988800L*/);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
		printf(" > FS_WriteFile: %lx\n", ret);
	}

	free(savedata);

	return ret;
}


Result Save_removeSecureValue(u8* ptr)
{
	printf("Save_removeSecureValue\n");

	Result ret;
	FS_MediaType mediaType;

	ret = FSUSER_GetMediaType(&mediaType);
	printf(" > FSUSER_GetMediaType: %lx\n", ret);
	if (R_FAILED(ret)) return ret;
	if (mediaType != MEDIATYPE_SD) return -1;

	u64 in = ((u64) SECUREVALUE_SLOT_SD << 32) | (_titleId & 0xFFFFFFFF);
	// u64 in = ((u64) SECUREVALUE_SLOT_SD << 32) | ((u32) _titleId);
	u8 out;

	ret = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &in, 8, &out, 1);
	printf(" > FSUSER_ControlSecureSave: %lx\n", ret);
	// if (R_FAILED(ret)) return ret;

	if (ptr) *ptr = out;
	printf("    > out: %d\n", out);

	return ret;
}
