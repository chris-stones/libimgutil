/*
 * internal.h
 *
 *  Created on: Oct 30, 2011
 *      Author: cds
 */

#ifndef INTERNAL_H_
#define INTERNAL_H_

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

void premultiply_alpha( struct imgPixel * pix );
void unpremultiply_alpha( struct imgPixel * pix ); 

struct imgPixel imgReadPacked(const struct imgImage *img, int x, int y);

struct imgPixel imgReadPlanar(const struct imgImage *img, int x, int y);

struct imgPixel imgReadCompressed(const struct imgImage *img, int x, int y);
	

void imgWritePacked(struct imgImage *img, int x, int y, struct imgPixel pix);

void imgWritePlanar(struct imgImage *img, int x, int y, struct imgPixel pix);
	
void imgWriteCompressed(struct imgImage *dstimg, const struct imgImage *srcimg);

static inline int need_to_pma(const struct imgImage *dst, const struct imgImage *src) {
  
  if( !(dst->format & IMG_FMT_COMPONENT_PMA) )
    return 0;
  
  return !(src->format & IMG_FMT_COMPONENT_PMA);
}

static inline int need_to_unpma(const struct imgImage *dst, const struct imgImage *src) {
  
  if(dst->format & IMG_FMT_COMPONENT_PMA)
    return 0;
  
  return (src->format & IMG_FMT_COMPONENT_PMA);
}


#ifdef __cplusplus
} /*** extern "C" { ***/
#endif

#endif /* INTERNAL_H_ */
