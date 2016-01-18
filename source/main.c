#include <3ds.h>
#include <stdio.h>

#include "fs.h"
#include "save_manager.h"

void waitKey(u32 key)
{
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & key)
			break;
	}
}

int main(int argc, char* argv[])
{
	gfxInitDefault();

	PrintConsole cmdConsole, logConsole;
	consoleInit(GFX_TOP, &cmdConsole);
	consoleInit(GFX_BOTTOM, &logConsole);

	FS_fsInit();

	if (!FS_IsInitialized())
	{
		printf("\nFS not fully initialized!\n");
		printf("Have you selected a title?\n\n");
	}
	else
	{
		printf("\nFS fully initialized!\n");
		printf("Good to go!\n\n");
	}

	if (Save_getTitleId(NULL))
	{
		printf("\nCouldn't get the title id!\n");
	}
	else
	{
		printf("\nTitle id well got!\n");
	}

	consoleSelect(&cmdConsole);
	printf("\n\n");
	printf("           Pokemon save manager\n");
	printf("\n\n\n\n");
	printf("    Commands:\n\n");
	printf("        \a Press X to export to SD\n\n");
	printf("        \a Press Y to import from SD\n\n");
	printf("        \a Press A to backup to SD\n\n");
	printf("        \a Press B to exit\n\n");
	printf("\n\n\n\n\n\n");
	printf("              Made for PCHex++ & PHBank\n\n");
	printf("                 By Gocario & Slashcash");
	consoleSelect(&logConsole);

	u32 kDown;
	while (aptMainLoop())
	{
		hidScanInput();

		kDown = hidKeysDown();

		if (kDown & KEY_SELECT)
		{
			// Shouldn't work, because of the FS_fsInit
			Save_removeSecureValue(NULL);
		}

		if (kDown & KEY_X)
		{
			printf("Exporting\n");
			Result ret = Save_exportSavedata();

			if (R_SUCCEEDED(ret))
			{
				printf("\nExportation was a success!\n\n");
			}
			else
			{
				printf("\nSomething mess up with the export!\n\n");
			}
		}

		if (kDown & KEY_Y)
		{
			printf("Importing\n");
			Result ret = Save_importSavedata();

			if (R_SUCCEEDED(ret))
			{
				printf("\nImportation was a success!\n\n");
			}
			else
			{
				printf("\nSomething mess up with the import!\n\n");
			}
		}

		if (kDown & KEY_A)
		{
			printf("Backing up\n");
			Result ret = Save_backupSavedata();

			if (R_SUCCEEDED(ret))
			{
				printf("\nThe backup was a success!\n\n");
			}
			else
			{
				printf("\nSomething mess up with the backup!\n\n");
			}
		}

		if (kDown & KEY_B)
			break;

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	FS_fsExit();

	Save_removeSecureValue(NULL);

	gfxExit();
	return 0;
}
