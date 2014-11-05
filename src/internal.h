/*
 * internal.h
 *
 *  Created on: Oct 30, 2011
 *      Author: cds
 */

#pragma once

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GL_GL_H
	#include <GL/gl.h>
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

void imgWriteCompressed(struct imgImage *dstimg, const struct imgImage *srcimg, copy_quality_t quality);

static inline int need_to_pma(const struct imgImage *dst,
		const struct imgImage *src) {

	if (!(dst->format & IMG_FMT_COMPONENT_PMA))
		return 0;

	return !(src->format & IMG_FMT_COMPONENT_PMA);
}

static inline int need_to_unpma(const struct imgImage *dst,
		const struct imgImage *src) {

	if (dst->format & IMG_FMT_COMPONENT_PMA)
		return 0;

	return (src->format & IMG_FMT_COMPONENT_PMA);
}

err_diffuse_kernel_t get_error_diffuse_kernel(err_diffuse_kernel_t k);

#ifdef __cplusplus
} /*** extern "C" { ***/
#endif

