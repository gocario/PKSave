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
 * @brief Returns the offset of the secure value depending on the bytes read.
 * @param bytesRead The number of bytes read.
 * @return The offset of the secure value.
 */
u32 Save_svOffset(u32 bytesRead);

/**
 * @brief Adds the secure value from the clean savedata to the savedata to fix.
 * @param savedata The savedata to fix.
 * @param svSavedata The clean savedata.
 * @param bytesRead The number of bytes read of the savedata to fix.
 * @param svBytesRead The number of bytes read of the cleaned savedata.
 * @return The error encountered.
 */
Result Save_fixSecureValue(u8* savedata, u8* svSavedata, u32 bytesRead, u32 svBytesRead);

#ifdef __cplusplus
}
#endif

#endif // SAVE_MANAGER_H
