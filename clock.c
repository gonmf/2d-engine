#include "common.h"
u32 get_millis_clock(){
	struct timespec tp;
	u32 ret = clock_gettime(CLOCK_REALTIME, &tp);
	assert(ret == 0);
	u32 current = (u32)(tp.tv_nsec / 1000000 + tp.tv_sec * 1000);
	return current;
}