#include "title_selector.h"

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

u32 saveLowPath[3];

Result launchTitleSelector(FS_Archive* archive)
{
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

	consoleClear();
		
	while (aptMainLoop())
	{
		printf("\x1B[0;0H  Title Selector");

		if (cartridge)
		{
			printf("\x1B[2;0HCartridge    ");
			printf("\x1B[3;0H               ");
		}
		else
		{
			printf("\x1B[2;0HGame: %s (%i) ", games[currentGame], currentGame);
			printf("\x1B[3;0HRegion: %s (%i)", regions[currentRegion], currentRegion);
		}

		printf("\x1B[5;0H X: swap cart/sdmc");
		printf("\x1B[6;0H Y: switch region");
		printf("\x1B[7;0H < >: roll game");

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
				}

				if (kDown & KEY_RIGHT)
				{
					currentGame = (currentGame + 1) % gameCount;
				}

				if (kDown & KEY_Y)
				{
					currentRegion = (currentRegion + 1) % regionCount;
				}
			}

			if (kDown & KEY_X)
			{
				cartridge = !cartridge;
			}

			if (kDown & KEY_B)
			{
				updated = false;
				break;
			}

			if (kDown & KEY_A)
			{
				updated = true;
				break;
			}
		}

		//Wait for VBlank
		gspWaitForVBlank();
	}

	printf("\n");

	if (updated)
	{
		u64 titleid = 0x000400000011C500;

		printf("\nArchive updated: ");

		if (cartridge)
		{
			printf("cartridge!\n\n");
			archive->id = ARCHIVE_GAMECARD_SAVEDATA;
			archive->lowPath = (FS_Path) { PATH_EMPTY, 0, NULL };
		}
		else
		{
			printf("eShop!\n\n");
			titleid = saveIds[currentGame][currentRegion];
			saveLowPath[0] = MEDIATYPE_SD;
			saveLowPath[1] = (u32) (titleid);
			saveLowPath[2] = (u32) (titleid >> 32);
			archive->id = ARCHIVE_USER_SAVEDATA;
			archive->lowPath = (FS_Path) { PATH_BINARY, 3 * 4, saveLowPath };
		}
	}
	else return 1;
	return 0;
}
