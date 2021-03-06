
#include "internal.h"

#if defined(_MSC_VER)
#define INLINE __inline
#else
#define INLINE inline
#endif

void premultiply_alpha(struct imgPixel * pix) {

	pix->red *= pix->alpha;
	pix->green *= pix->alpha;
	pix->blue *= pix->alpha;
}

static INLINE void unpma(float * f, float a) {

	if (a < (1.0f / 255.0f))
		*f = 0.0f;
	else if (a < (254.0f / 255.0f))
		*f = *f / a;
}

void unpremultiply_alpha(struct imgPixel * pix) {

	unpma(&pix->red, pix->alpha);
	unpma(&pix->green, pix->alpha);
	unpma(&pix->blue, pix->alpha);
}

