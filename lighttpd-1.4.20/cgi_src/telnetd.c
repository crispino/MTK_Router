#include <stdio.h>
#include "utils.h"

int main(int argc, char *argv[])
{
	DBG_MSG("Telnetd start!");

	do_system("killall telnetd");

	do_system("telnetd &");

	printf("Telnetd start!");

	return 0;
}

