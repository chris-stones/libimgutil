/*
 * imgutil.c
 *
 *  Created on: Oct 29, 2011
 *      Author: cds
 */


#include "internal.h"

#include<limits.h>
#include<assert.h>
#include<stdlib.h>
#include<string.h>

int imguCopyImage(struct imgImage *dst, const struct imgImage *src) {

  if( dst->format & IMG_FMT_COMPONENT_COMPRESSED ) {
    
    imgWriteCompressed( dst, src );
    return 0;
  }
  
  return imguCopyRect( dst,src,0,0,0,0,src->width, src->height );
}

int imguCopyPixel(struct imgImage *dst, const struct imgImage *src, int dx, int dy, int sx, int sy) {

	return imguCopyRect(
			dst,src,dx,dy,sx,sy,1,1);
}

typedef struct imgPixel(*imguReadPixel_fptr)	(const struct imgImage*,int,int);
typedef        void    (*imguWritePixel_fptr)	(struct imgImage*,int,int,struct imgPixel);

static imguReadPixel_fptr GetPixelReader(enum imgFormat fmt) {

	if(fmt & IMG_FMT_COMPONENT_COMPRESSED)
		return &imgReadCompressed;
  
	if(fmt & IMG_FMT_COMPONENT_PLANAR)
		return &imgReadPlanar;

	return &imgReadPacked;
}

static imguWritePixel_fptr GetPixelWriter(enum imgFormat fmt) {

	if(fmt & IMG_FMT_COMPONENT_COMPRESSED) {
	  assert(0 && "cannot perform per-pixel writes on compressed images");
	}
  
	if(fmt & IMG_FMT_COMPONENT_PLANAR)
		return &imgWritePlanar;

	return &imgWritePacked;
}


static struct imgImage *onePixel(enum imgFormat fmt) {

	struct imgImage *img;

	if(imgAllocImage(&img) != IMG_OKAY)
		return NULL;

	img->format = fmt;
	img->width = img->height = 1;

	if(imgAllocPixelBuffers(img) != IMG_OKAY) {

		imgFreeImage(img);
		return (NULL);
	}

	return img;
}

int imguCopyRect(struct imgImage *dst, const struct imgImage *src, int dx, int dy, int sx, int sy, int w, int h) {

	imguReadPixel_fptr 	imguReadPixel 	= GetPixelReader(src->format);
	imguWritePixel_fptr imguWritePixel	= GetPixelWriter(dst->format);

	if(!src || !dst)
		return IMGU_ERROR;
	if(dx<0 || dy<0 || sx<0 || sy<0 || w<0 || h<0)
		return IMGU_OUT_OF_BOUNDS;
	if((dx+w)>dst->width || (dy+h)>dst->height)
		return IMGU_OUT_OF_BOUNDS;
	if((sx+w)>src->width || (sy+h)>src->height)
		return IMGU_OUT_OF_BOUNDS;

	if( 1 && (src->format & IMG_FMT_COMPONENT_PACKED) && (dst->format & IMG_FMT_COMPONENT_420P)) {

		//  special case, CbCr needs to be sampled at half YA resolution.

		// TODO: Handle special-case premultiplied alpha here ?
		
		int x,y,ah,aw;
		struct imgPixel pixels[2][2];

		struct imgImage *p = onePixel(dst->format);

		if(!p)
			return IMG_ERROR;

		// adjusted width / height - can't sample edge pixels on odd sized images in the usual way
		aw = ((src->width &1) && ((sx+w) == src->width )) ? (w-1) : w;
		ah = ((src->height&1) && ((sy+h) == src->height)) ? (h-1) : h;

		for(y=0; y<ah; y++)
			for(x=0;x<aw;x++) {

				// get top left of source 4x4 for CrCb sample
				int st = (sy+y) & (~(int)1);
				int sl = (sx+x) & (~(int)1);

				// get destination pixel addresses
				struct imgData dpx = imgGetPixel(dst,dx+x,dy+y);

				// sample source 4x4 CrCb
				pixels[0][0] = imguReadPixel(src, sl+0, st+0);
				pixels[0][1] = imguReadPixel(src, sl+1, st+0);
				pixels[1][0] = imguReadPixel(src, sl+0, st+1);
				pixels[1][1] = imguReadPixel(src, sl+1, st+1);

				// take an average.
				pixels[0][0].red   += pixels[0][1].red   + pixels[1][0].red   + pixels[1][1].red;
				pixels[0][0].green += pixels[0][1].green + pixels[1][0].green + pixels[1][1].green;
				pixels[0][0].blue  += pixels[0][1].blue  + pixels[1][0].blue  + pixels[1][1].blue;
				pixels[0][0].alpha += pixels[0][1].alpha + pixels[1][0].alpha + pixels[1][1].alpha;
				pixels[0][0].red   /= 4.0f;
				pixels[0][0].green /= 4.0f;
				pixels[0][0].blue  /= 4.0f;
				pixels[0][0].alpha /= 4.0f;

				// write YCbCrA
				imguWritePixel(dst, dx+x, dy+y,
					imguReadPixel(src, sx+x, sy+y));

				// convert average to YCbCrA 420P
				imguWritePixel(p,0,0,pixels[0][0]);

				// Overwrite destination CbCr
				*(char*)(dpx.channel[1]) = *(char*)p->data.channel[1];
				*(char*)(dpx.channel[2]) = *(char*)p->data.channel[2];
			}

		// sample right edge odd-width pixels.
		if(1 && (w!=aw))
			for(y=0;y<ah;y++) {

				int st = (sy+y) & (~(int)1);
				int sl = (sx+w-1);
				int  x = w-1;

				// get destination pixel addresses
				struct imgData dpx = imgGetPixel(dst,dx+x,dy+y);

				pixels[0][0] = imguReadPixel(src, sl+0, st+0);
				pixels[1][0] = imguReadPixel(src, sl+0, st+1);

				pixels[0][0].red   += pixels[1][0].red;
				pixels[0][0].green += pixels[1][0].green;
				pixels[0][0].blue  += pixels[1][0].blue;
				pixels[0][0].alpha += pixels[1][0].alpha;
				pixels[0][0].red   /= 2.0f;
				pixels[0][0].green /= 2.0f;
				pixels[0][0].blue  /= 2.0f;
				pixels[0][0].alpha /= 2.0f;

				// write YCbCrA
				imguWritePixel(dst, dx+x, dy+y,
					imguReadPixel(src, sx+x, sy+y));

				// convert average to YCbCrA 420P
				imguWritePixel(p,0,0,pixels[0][0]);

				// Overwrite destination CbCr
				*(char*)(dpx.channel[1]) = *(char*)p->data.channel[1];
				*(char*)(dpx.channel[2]) = *(char*)p->data.channel[2];
			}

		// sample bottom edge odd-height pixels.
		if(1 && (h!=ah))
			for(x=0;x<aw;x++) {

				int st = sy+h-1;
				int sl = (sx+x) & (~(int)1);
				int y  = h-1;

				// get destination pixel addresses
				struct imgData dpx = imgGetPixel(dst,dx+x,dy+y);

				pixels[0][0] = imguReadPixel(src, sl+0, st+0);
				pixels[0][1] = imguReadPixel(src, sl+1, st+0);

				pixels[0][0].red   += pixels[0][1].red;
				pixels[0][0].green += pixels[0][1].green;
				pixels[0][0].blue  += pixels[0][1].blue;
				pixels[0][0].alpha += pixels[0][1].alpha;
				pixels[0][0].red   /= 2.0f;
				pixels[0][0].green /= 2.0f;
				pixels[0][0].blue  /= 2.0f;
				pixels[0][0].alpha /= 2.0f;

				// write YCbCrA
				imguWritePixel(dst, dx+x, dy+y,
					imguReadPixel(src, sx+x, sy+y));

				// convert average to YCbCrA 420P
				imguWritePixel(p,0,0,pixels[0][0]);

				// Overwrite destination CbCr
				*(char*)(dpx.channel[1]) = *(char*)p->data.channel[1];
				*(char*)(dpx.channel[2]) = *(char*)p->data.channel[2];
			}

		// write bottom-right edge odd-width-and-height pixels.
		if(1 && h!=ah && w != aw) {

			int x  = w-1;
			int y  = h-1;
			imguWritePixel(dst, dx+x, dy+y, imguReadPixel(src, sx+x, sy+y));
		}

		imgFreeAll(p);

	}
	else {

		/* generic convert */
		
		int x,y;
		struct imgPixel pix;
		
		if( need_to_pma( dst, src) ) {
		  
		  for(y=0;y<h;y++)
		    for(x=0;x<w;x++) {
		      
		      pix = imguReadPixel(src,sx+x,sy+y);
		      
		      premultiply_alpha(&pix);
		      
		      imguWritePixel(dst,dx+x,dy+y,pix);
		    }
		  
		} else if( need_to_unpma( dst, src ) ) {
		  
		  for(y=0;y<h;y++)
		    for(x=0;x<w;x++) {
		      
		      pix = imguReadPixel(src,sx+x,sy+y);
		      
		      unpremultiply_alpha(&pix);
		      
		      imguWritePixel(dst,dx+x,dy+y,pix);
		    }
		  
		} else if( (src->format & IMG_FMT_COMPONENT_PACKED) && (src->format == dst->format ) ) {
		  
		  // same packed source and destination formats - coopy raw pixel data.
		  
		  struct imgData src_data;
		  struct imgData dst_data;
		  
		  for(y=0;y<h;y++)
		    for(x=0;x<w;x++) {
		      src_data = imgGetPixel(src,sx+x, sy+y);
		      dst_data = imgGetPixel(dst,dx+x, dy+y);
		      memcpy( dst_data.channel[0], src_data.channel[0], imgGetBytesPerPixel(src->format, 0) );
		    }
		  
		} else {
	
		  // generic copy
		  for(y=0;y<h;y++)
		    for(x=0;x<w;x++)
		      imguWritePixel(dst,dx+x,dy+y,
			imguReadPixel(src,sx+x,sy+y));
		}
	}

	return IMGU_OKAY;
}










