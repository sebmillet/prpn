// Compat.cpp
// Compatibility functions

// RPN calculator

// Sébastien Millet
// August 2009, May 2010

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef HAVE_SLEEP
#include <unistd.h>
void my_sleep_seconds(const int& n) { sleep(n); }
#else // ! HAVE_SLEEP
#include <time.h>
void my_sleep_seconds(const int& n) {
	clock_t endsleep = endsleep = clock() + n * CLOCKS_PER_SEC;
	while (clock() < endsleep) {}
}
#endif


