#pragma once
/**
 * @file fs.h
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
 * @brief Returns the offset of the secure value depending on the size in bytes.
 * @param bytesRead The size of the save in bytes.
 * @return The offset of the secure value.
 * @deprecated
 * @see Save_fixSecureValue
 */
u32 Save_svOffsetSize(u32 size);

/**
 * @brief Adds the secure value from the clean savedata to the savedata to fix.
 * @param savedata The savedata to fix.
 * @param svSavedata The clean savedata.
 * @param bytesRead The number of bytes read of the savedata to fix.
 * @param svBytesRead The number of bytes read of the cleaned savedata.
 * @return The error encountered.
 * @deprecated
 * @see Save_fixSecureValue
 */
Result Save_fixSecureValue(u8* savedata, u8* svSavedata, u32 bytesRead, u32 svBytesRead);

/**
 * @brief Retrieves the size of the savedata of a title based on its id.
 * @param titleId Id of the title.
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
