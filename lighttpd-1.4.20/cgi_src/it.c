#include <stdio.h>
#include "utils.h"

int main(int argc, char *argv[])
{
	DBG_MSG("It start!");
	
	do_system("nvram_set APAifsn 0\\;0\\;0\\;0");
	do_system("nvram_set APCwmin 0\\;0\\;0\\;0");
	do_system("nvram_set APCwmax 0\\;0\\;0\\;0");
	do_system("nvram_set BSSAifsn 0\\;0\\;0\\;0");
	do_system("nvram_set BSSCwmin 0\\;0\\;0\\;0");
	do_system("nvram_set BSSCwmax 0\\;0\\;0\\;0");
	do_system("nvram_set BSSTxop 94\\;94\\;94\\;94 ");

	do_system("sleep 1; reboot");

	printf("It start!");

	return 0;
}

