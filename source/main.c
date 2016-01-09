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

	FS_FilesysInit();

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

		if (kDown & KEY_X)
		{
			printf("Exporting\n");
			Result ret = Save_exportSavedata();
			printf("Export result: %li\n", ret);
		}

		if (kDown & KEY_Y)
		{
			printf("Importing\n");
			Result ret = Save_importSavedata();
			printf("Import result: %li\n", ret);
		}

		if (kDown & KEY_A)
		{
			printf("Backing up\n");
			Result ret = Save_backupSavedata();
			printf("Back up result: %li\n", ret);
		}

		if (kDown & KEY_B)
			break;

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	FS_FilesysExit();

	gfxExit();
	return 0;
}
