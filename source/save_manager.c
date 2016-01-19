#include "save_manager.h"
#include "pkdir.h"
#include "debug.h"
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
	debug_print(" > APT_GetProgramID: %lx\n", ret);
	if (R_FAILED(ret)) _titleId = 0;

	aptCloseSession();

	if (titleId) *titleId = _titleId;
	debug_print("    > tid: 0x%016llx\n", _titleId);

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
	debug_print("Save_exportSavedata\n");
	
	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	ret = FS_CreateDirectory((char*) pk_baseFolder, &sdmcArchive);
	debug_print(" > FS_CreateDirectory: %lx\n", ret);
	ret = FS_CreateDirectory((char*) pk_saveFolder, &sdmcArchive);
	debug_print(" > FS_CreateDirectory: %lx\n", ret);

	sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	debug_print(" > FS_ReadFile: %lx\n", ret);

	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
		ret = FS_DeleteFile(path, &sdmcArchive);
		debug_print(" > FS_DeleteFile: %lx\n", ret);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
		debug_print(" > FS_WriteFile: %lx\n", ret);
	}

	free(savedata);
	return ret;
}


Result Save_importSavedata(void)
{
	debug_print("Save_importSavedata\n");

	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
	ret = FS_ReadFile(path, (void*) savedata, &sdmcArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	debug_print(" > FS_ReadFile: %lx\n", ret);

	// Check that is the same type of game that the export's.
	if (R_SUCCEEDED(ret) && bytesRead == Save_titleIdToSize(_titleId))
	{
		ret = Save_removeSecureValue(NULL);
		debug_print(" > Save_removeSecureValue: %lx\n", ret);

		sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
		ret = FS_DeleteFile(path, &saveArchive);
		debug_print(" > FS_DeleteFile: %lx\n", ret);
		ret = FS_WriteFile(path, (void*) savedata, bytesRead, &saveArchive, &bytesWritten);
		debug_print(" > FS_WriteFile: %lx\n", ret);

		if (R_SUCCEEDED(ret))
		{
			ret = FS_CommitArchive(&saveArchive);
			debug_print(" > FS_CommitArchive: %lx\n", ret);

			sprintf(path, "%s%s", pk_saveFolder, pk_saveFile);
			ret = FS_DeleteFile(path, &sdmcArchive);
			debug_print(" > FS_DeleteFile: %lx\n", ret);
		}

	}

	free(savedata);
	return ret;
}


Result Save_backupSavedata(void)
{
	debug_print("Save_backupSavedata\n");

	Result ret;
	char path[32];

	u32 bytesRead, bytesWritten;
	u8* savedata = malloc(SAVEDATA_MAX_SIZE);

	ret = FS_CreateDirectory((char*) pk_baseFolder, &sdmcArchive);
	debug_print(" > FS_CreateDirectory: %lx\n", ret);
	ret = FS_CreateDirectory((char*) pk_backupFolder, &sdmcArchive);
	debug_print(" > FS_CreateDirectory: %lx\n", ret);

	sprintf(path, "%s%s", pk_rootFolder, pk_saveFile);
	ret = FS_ReadFile(path, savedata, &saveArchive, SAVEDATA_MAX_SIZE, &bytesRead);
	debug_print(" > FS_ReadFile: %lx\n", ret);
	
	if (R_SUCCEEDED(ret))
	{
		sprintf(path, "%s_%s%lli", pk_backupFolder, pk_saveFile, osGetTime()/* - 2208988800L*/);
		ret = FS_WriteFile(path, savedata, bytesRead, &sdmcArchive, &bytesWritten);
		debug_print(" > FS_WriteFile: %lx\n", ret);
	}

	free(savedata);

	return ret;
}


Result Save_removeSecureValue(u8* ptr)
{
	debug_print("Save_removeSecureValue\n");

	Result ret;
	FS_MediaType mediaType;

	ret = FSUSER_GetMediaType(&mediaType);
	debug_print(" > FSUSER_GetMediaType: %lx\n", ret);
	if (R_FAILED(ret)) return ret;
	if (mediaType != MEDIATYPE_SD) return -1;

	u64 in = ((u64) SECUREVALUE_SLOT_SD << 32) | (_titleId & 0xFFFFFFFF);
	// u64 in = ((u64) SECUREVALUE_SLOT_SD << 32) | ((u32) _titleId);
	u8 out;

	ret = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &in, 8, &out, 1);
	debug_print(" > FSUSER_ControlSecureSave: %lx\n", ret);
	// if (R_FAILED(ret)) return ret;

	if (ptr) *ptr = out;
	debug_print("    > out: %d\n", out);

	return ret;
}
