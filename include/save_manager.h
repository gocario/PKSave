#pragma once
/**
 * @file save_manager.h
 * @brief Save Manager
 */
#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <3ds/types.h>

/**
 * @brief Retrieves some data of the current process.
 * @return The error encountered.
 */
Result Save_getTitleId(void);

/**
 * @brief Retrieves some data of the current process.
 * @return The error encountered.
 */
Result Save_getMediaType(void);

/**
 * @brief Exports the savedata from the card/title to the SD.
 * @return The error encountered.
 */
Result Save_exportSavedata(void);

/**
 * @brief Imports the savedata from the SD to the card/title.
 * @return The error encountered.
 */
Result Save_importSavedata(void);

/**
 * @brief Backs up the savedata from the card/title to the SD.
 * @return The error encountered.
 */
Result Save_backupSavedata(void);

/**
 * @brief Checks if a title id is a Pokémon title id.
 * @param titleId The id of the title.
 * @return Whether the title is a Pokémon title.
 */
bool Save_titleIdIsPokemon(u64 titleId);

/**
 * @brief Retrieves the size of the savedata of a title based on its id.
 * @param titleId The id of the title.
 * @return The size of the savedata of the tile.
 */
u32 Save_titleIdToSize(u64 titleId);

/**
 * @brief remove the secure value of the title in the NAND.
 * @return The error encountered.
 */
Result Save_removeSecureValue();

#ifdef __cplusplus
}
#endif

#endif // SAVE_MANAGER_H
