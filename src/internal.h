/*
 * internal.h
 *
 *  Created on: Oct 30, 2011
 *      Author: cds
 */

#pragma once

#ifdef HAVE_CONFIG_H
	#include <config.h>
#else
	// NOT AN AUTOTOOLS BUILD... setup some sane defaults...
	#define HAVE_GL_GL_H 1
#endif

#ifdef HAVE_GL_GL_H
	#if defined(_MSC_VER)
		#include<Windows.h>
	#endif
	#include <GL/gl.h>
#endif

#if defined(_MSC_VER)
	#define INLINE __inline
#else
	#define INLINE inline
#endif

#include "libimgutil.h"

#ifdef __cplusplus
extern "C" {
#endif

struct imgPixel {

	float red;
	float green;
	float blue;
	float alpha;
};

void premultiply_alpha(struct imgPixel * pix);
void unpremultiply_alpha(struct imgPixel * pix);

struct imgPixel imgReadPacked(const struct imgImage *img, int x, int y);

struct imgPixel imgReadPlanar(const struct imgImage *img, int x, int y);

struct imgPixel imgReadCompressed(const struct imgImage *img, int x, int y);

void imgWritePacked(struct imgImage *img, int x, int y, struct imgPixel pix);

void imgWritePlanar(struct imgImage *img, int x, int y, struct imgPixel pix);

int imgWriteCompressed(struct imgImage *dstimg, const struct imgImage *srcimg, copy_quality_t quality);

static INLINE int need_to_pma(const struct imgImage *dst,
		const struct imgImage *src) {

	if (!(dst->format & IMG_FMT_COMPONENT_PMA))
		return 0;

	return !(src->format & IMG_FMT_COMPONENT_PMA);
}

static INLINE int need_to_unpma(const struct imgImage *dst,
		const struct imgImage *src) {

	if (dst->format & IMG_FMT_COMPONENT_PMA)
		return 0;

	return (src->format & IMG_FMT_COMPONENT_PMA);
}

err_diffuse_kernel_t get_error_diffuse_kernel(err_diffuse_kernel_t k);

#ifdef __cplusplus
} /*** extern "C" { ***/
#endif

