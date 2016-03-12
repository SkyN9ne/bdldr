
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "bdldr.h"

int main(int argc, char *argv[])
{
	void *bdlib, *core, *instance;
	_CoreInit4 CoreInit4;
	_CoreNewInstance CoreNewInstance;
	_CoreDeleteInstance CoreDeleteInstance;
	_CoreUninit CoreUninit;
	_CoreGet CoreGet;
	_CoreSet CoreSet;

	if (argc != 2) {
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "%s sample.exe\n", argv[0]);
		return EXIT_FAILURE;
	}

	bdlib = dlopen(BD_CORE_PATH, RTLD_NOW);
	if ( bdlib != NULL ) {

		fprintf(stderr, "Initializing BD core ...\n");

		/* Init BD Core */
		*(void **)(&CoreInit4) = dlsym(bdlib, "CoreInit4");
		core = CoreInit4(BD_PLUGINS_PATH);

		*(void **)(&CoreNewInstance) = dlsym(bdlib, "CoreNewInstance");
		*(void **)(&CoreDeleteInstance) = dlsym(bdlib, "CoreDeleteInstance");
		*(void **)(&CoreUninit) = dlsym(bdlib, "CoreUninit");

		fprintf(stderr, "Initializing BD core instance ...\n");
		instance = CoreNewInstance();

		*(void **)(&CoreGet) = dlsym(bdlib, "CoreGet");
		*(void **)(&CoreSet) = dlsym(bdlib, "CoreSet");

		fprintf(stderr, "Setting up instance ...\n");
		CoreSet(instance, BD_ACTION, BD_ACTION_IGNORE, CoreSet);
		CoreSet(instance, BD_HEURISTICS, BD_ENABLE, CoreSet);
		CoreSet(instance, BD_EXE_UNPACK, BD_ENABLE, CoreSet);
		CoreSet(instance, BD_ARCHIVE_UNPACK, BD_ENABLE, CoreSet);
		CoreSet(instance, BD_EMAIL_UNPACK, BD_ENABLE, CoreSet);

		/* scan file */
		fprintf(stderr, "Scanning file ...\n");
		CoreSet(instance, BD_SCAN, NULL, argv[1]);

		fprintf(stderr, "Deleting BD core instance ...\n");
		CoreDeleteInstance(instance);
		fprintf(stderr, "Deleting BD core ...\n");
		CoreUninit(core);

	} else {
		fprintf(stderr, "Error loading bdcore.so\n");
		return EXIT_FAILURE;
	}
	if (bdlib != NULL ) dlclose(bdlib);
	return EXIT_SUCCESS;
}

