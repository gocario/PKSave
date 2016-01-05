#include "fs.h"

#include <3ds/services/fs.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/svc.h>

Handle* fsHandle;
FS_Archive sdmcArchive;
FS_Archive saveArchive;
