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
 * @brief Export the savedata from the card/title to the SD.
 * @return The error encountered.
 */
Result Save_exportSavedata(void);

/**
 * @brief Import the savedata from the SD to the card/title.
 * @return The error encountered.
 */
Result Save_importSavedata(void);

/**
 * @brief Backup the savedata from the card/title to the SD.
 * @return The error encountered.
 */
Result Save_backupSavedata(void);

#ifdef __cplusplus
}
#endif

#endif // SAVE_MANAGER_H
