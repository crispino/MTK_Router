
#include <time.h>

#include "ont/platform.h"
#include "ont/error.h"

extern void usleep(int micro_seconds);

int32_t ont_platform_time()
{
    time_t sec;
    time(&sec);
    return (int32_t)sec;   
}

struct timeval {
	time_t	tv_sec;		/* seconds */
	time_t	tv_usec;	/* and microseconds */
};

void ont_platform_sleep(int milliseconds)
{
	milliseconds = milliseconds<30?30:milliseconds;
	struct timeval tval;
	tval.tv_sec=milliseconds/1000;
	tval.tv_usec=(milliseconds*1000)%1000000;
	select(0,NULL,NULL,NULL,&tval);

	/*cyg_thread_delay(100)	one second */
	//cyg_thread_delay( milliseconds/10 );
}


