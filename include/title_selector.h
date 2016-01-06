#pragma once
/**
 * @file title_selector.hpp
 * @brief Title selector service.
 */
#include "fs.h"

/**
 * @brief Prompt the data structure.
 * @param archive The archive to return.
 * @return The error encountered.
 */
Result launchTitleSelector(FS_Archive* archive);
