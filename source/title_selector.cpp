#include "title_selector.h"

#include <3ds.h>
#include <stdio.h>

FS_Archive launchTitleSelector(void)
{
	FS_Archive archive;

	const u8 gameCount = 4;
	const char* games[] = {
		"X",
		"Y",
		"OR",
		"AS",
	};

	const u8 regionCount = 4;
	const char* regions[] = {
		"JPN",
		"USA",
		"EUR",
		"---",
	};

	const u16 saveIds[gameCount][regionCount] = {
		{
			// Pokémon X
			0x0, // JPN
			0x0, // USA
			0x0, // EUR
			0x0, // ---
		},
		{
			// Pokémon Y
			0x0, // JPN
			0x0, // USA
			0x0, // EUR
			0x0, // ---
		},
		{
			// Pokémon OR
			0x0, // JPN
			0x0, // USA
			0x0, // EUR
			0x0, // ---
		},
		{
			// Pokémon AS
			0x0, // JPN
			0x0, // USA
			0x0, // EUR
			0x0, // ---
		}
	};

	bool updated = false;
	bool cartridge = true;
	s8 currentRegion = 0;
	s8 currentGame = 0;

	while (aptMainLoop())
	{
		consoleClear();
		
		printf("  Title Selector\n\n");

		if (cartridge)
		{
			printf("Cartridge\n");
		}
		else
		{
			printf("Game: %s (%i)\n", games[currentGame], currentGame);
			printf("Region: %s (%i)\n", regions[currentRegion], currentRegion);
		}

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		{
			hidScanInput();

			u32 kDown = hidKeysDown();

			if (!cartridge)
			{
				if (kDown & KEY_LEFT)
				{
					currentGame = (currentGame - 1 + gameCount) % gameCount;
					updated = true;
				}

				if (kDown & KEY_RIGHT)
				{
					currentGame = (currentGame + 1) % gameCount;

					updated = true;
				}

				if (kDown & KEY_Y)
				{
					currentRegion = (currentRegion + 1) % regionCount;
					updated = true;
				}
			}

			if (kDown & KEY_X)
			{
				cartridge = !cartridge;
				updated = true;
			}

			if (kDown & KEY_START)
				break;
		}

		//Wait for VBlank
		gspWaitForVBlank();
	}

	if (!updated)
	{
		if (cartridge)
		{
			// TODO Change the "new"
			void* pathData = new FS_UserSaveData_PathData({ MEDIATYPE_GAME_CARD, 0, 0 });
			archive.id = ARCHIVE_GAMECARD_SAVEDATA;
			archive.lowPath = fsMakePath(PATH_BINARY, pathData);
			archive.handle = 0;
		}
		else
		{
			// TODO Change the "new"
			void* pathData = new FS_UserSaveData_PathData({ MEDIATYPE_SD, (u8) (saveIds[currentGame][currentRegion]), (u8) (saveIds[currentGame][currentRegion] >> 8) });
			archive.id = ARCHIVE_USER_SAVEDATA;
			archive.lowPath = fsMakePath(PATH_BINARY, pathData);
			archive.handle = 0;
		}

		updated = true;
	}

	return archive;
}
