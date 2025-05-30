#include <unistd.h>
#include <stdio.h>
#include <signal.h>

extern char **environ;

void
handle_sigterm(int sig) {
    dprintf(1, "SIGTERM received, terminating program...\n");
	// _exit(0);
}

void
handle_sigint(int sig) {
    dprintf(1, "SIGINT received!!!\n");
}

int
main()
{
	dprintf(1, "Starting infinit living process\n");
	signal(SIGTERM, handle_sigterm);
	signal(SIGINT, handle_sigint);

	// for (size_t i = 0; i < 5; i++)
	// {
	// 	dprintf(1, "I'm alive\n");
	// 	// fflush(stdout);
	// 	sleep(1);
	// }

	for (char **env = environ; *env != NULL; env++) {
		dprintf(1, "%s\n", *env);
	}

	while (1)
	{
		dprintf(1, "I'm alive\n");
		// ddprintf(1, 2, "I'm alive\n");
		// fflush(stdout);
		sleep(1);
	}
	dprintf(1, "I'm dead\n");
	return 0;
}