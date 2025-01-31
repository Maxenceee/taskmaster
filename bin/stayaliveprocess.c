#include <unistd.h>
#include <stdio.h>
#include <signal.h>

void
handle_sigterm(int sig) {
    dprintf(2, "SIGTERM received, terminating program...\n");
	_exit(0);
}

void
handle_sigint(int sig) {
    dprintf(2, "SIGINT received!!!\n");
}

int
main()
{
	printf("Starting infinit living process\n");
	// signal(SIGTERM, handle_sigterm);
	// signal(SIGINT, handle_sigint);
	// for (size_t i = 0; i < 5; i++)
	// {
	// 	printf("I'm alive\n");
	// 	// fflush(stdout);
	// 	sleep(1);
	// }
	while (1)
	{
		printf("I'm alive\n");
		// dprintf(2, "I'm alive\n");
		// fflush(stdout);
		sleep(1);
	}
	printf("I'm dead\n");
	return 0;
}