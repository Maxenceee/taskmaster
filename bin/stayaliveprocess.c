#include <unistd.h>
#include <stdio.h>

int
main()
{
	printf("Starting infinit living process\n");
	// for (size_t i = 0; i < 5; i++)
	// {
	// 	printf("I'm alive\n");
	// 	sleep(1);
	// }
	while (1)
	{
		printf("I'm alive\n");
		sleep(1);
	}
	printf("I'm dead\n");
	return 0;
}