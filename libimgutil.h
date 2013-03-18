/*
 * libimgutil.h
 *
 *  Created on: Oct 29, 2011
 *      Author: cds
 */

#ifndef LIBIMGUTIL_H_
#define LIBIMGUTIL_H_

#include<libimg.h>

#define IMGU_OKAY IMG_OKAY
#define IMGU_ERROR -1
#define IMGU_OUT_OF_BOUNDS -2

#ifdef __cplusplus
extern "C" {
#endif

int imguCopyImage	(struct imgImage *dst, const struct imgImage *src);
int imguCopyPixel	(struct imgImage *dst, const struct imgImage *src, int dx, int dy, int sx, int sy);
int imguCopyRect	(struct imgImage *dst, const struct imgImage *src, int dx, int dy, int sx, int sy, int w, int h);

int imguRotateCW	(struct imgImage **rotated, const struct imgImage *src);
int imguPad		(struct imgImage **padded, const struct imgImage *src, int top, int bot, int left, int right);

#ifdef __cplusplus
} /*** extern "C" { ***/
#endif

#endif /* LIBIMGUTIL_H_ */
