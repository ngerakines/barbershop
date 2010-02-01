
#ifndef __STATS_H__
#define __STATS_H__

#include <time.h>

struct _app_stats {
	time_t started_at;
	char *version;
	// Number of updates receieved
	unsigned int updates;
	// Number of items created/inserted into `items`
	unsigned int items;
	// Number of items created/inserted into `items` that reflects successful `next` calls
	unsigned int items_gc;
	// Number of created pools
	unsigned int pools;
	// Number of pools that reflects successful `next` calls that empty pools
	unsigned int pools_gc;
} app_stats;

#endif
