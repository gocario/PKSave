#include <3ds.h>

#include "title_selector.h"

int main(int argc, char* argv[])
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	launchTitleSelector();

	gfxExit();
	return 0;
}
