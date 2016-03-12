
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bdldr.h"

#define DAEMONIZE	0

static void daemonize(void)
{
	pid_t pid, sid;

	/* already a daemon */
	if ( getppid() == 1 ) return;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* At this point we are executing as the child process */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory.  This prevents the current
	   directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}

	/* Redirect standard files to /dev/null */
	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);
}

int main(int argc, char *argv[])
{
	void *bdlib;
	void *core, *instance;
	_CoreInit4 CoreInit4;
	_CoreNewInstance CoreNewInstance;
	_CoreDeleteInstance CoreDeleteInstance;
	_CoreUninit CoreUninit;
	_CoreGet CoreGet;
	_CoreSet CoreSet;

	struct stat sb;

	/* socket server */
	char * socket_path = "/tmp/bdldr.sock", socket_buffer[1024];
	struct sockaddr_un addr;
	int fd, cl, rc = 1;
	char * ptr;

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

		if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("socket error");
			exit(EXIT_FAILURE);
		}
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
		unlink(socket_path);
		if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
			perror("bind error");
			exit(EXIT_FAILURE);
		}
		if (listen(fd, 5) == -1) {
			perror("listen error");
			exit(EXIT_FAILURE);
		}
		chmod(socket_path, 0777);
		fprintf(stderr, "Waiting for connections at %s\n", socket_path);

		/* Ignore SIGPIPE */
		signal(SIGPIPE, SIG_IGN);

#if DAEMONIZE
		fprintf(stderr, "Daemonizing ...\n");
		daemonize();
#endif

		for (;;) {
			if ((cl = accept(fd, NULL, NULL)) == -1) {
				perror("accept error");
				continue;
			}

			/* Read only 1 command */
			rc = read(cl, socket_buffer, sizeof(socket_buffer));

			if (rc > 0) {
				if (strstr(socket_buffer, "SCAN ")) {
					ptr = strchr(socket_buffer, '\n');
					*ptr = 0x00;
					ptr = strchr(socket_buffer, ' ') + 1;
					// Check file exists
					if (stat(ptr, &sb) == -1) {
						write(cl, "NO FILE", 7);
						close(cl);
						continue;
					}

					fprintf(stderr, "[%d] Scanning %s ...\n", getpid(), ptr);

					/* scan file */
					/* handle hangs when fuzzing */
					alarm(60);
					CoreSet(instance, BD_SCAN, NULL, ptr);
					alarm(0);
					write(cl, "OUTPUT OK", 9);
				}
			}
			close(cl);
		};
		CoreDeleteInstance(instance);
		CoreUninit(core);
	} else {
		fprintf(stderr, "Error loading bdcore.so\n");
		return EXIT_FAILURE;
	}
	if (bdlib != NULL) dlclose(bdlib);
	return EXIT_SUCCESS;
}

