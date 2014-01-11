#include "internal.h"

// TODO: a real hash function.
static unsigned int pl_hash(const char* _s, size_t len, unsigned int seed) {
	const char * s = _s;
	unsigned int hash = seed;
	int i;

	// THANKS PAUL LARSON.
	for (i = 0; i < len; i++)
		hash = hash * 101 + *s++;

	return hash;
}

int imguBinaryHash32(struct imgImage *a) {

	int hash = 0xB16B00B5;
	int channel;

	for (channel = 0; channel < 4; channel++)
		hash = pl_hash(a->data.channel[channel], a->linearsize[channel], hash);

	return hash;
}

