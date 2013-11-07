/*
 * packed_pixel_ops.c
 *
 *  Created on: Oct 30, 2011
 *      Author: cds
 */

#include "internal.h"

#include<assert.h>

static int packed_offset(enum imgFormat fmt, int component_bit) {

	static const int tbl[4][4] = {
		// R  G  B  A
		{  0, 1, 2, 3 }, // RGBA
		{  1, 2, 3, 0 }, // ARGB
		{  2, 1, 0, 3 }, // BGRA
		{  3, 2, 1, 0 }, // ABGR
	};

	int order=0;
	int colour=0;

	switch(component_bit) {
		case IMG_FMT_COMPONENT_RED: 	colour = 0; break;
		case IMG_FMT_COMPONENT_GREEN:	colour = 1; break;
		case IMG_FMT_COMPONENT_BLUE: 	colour = 2; break;
		case IMG_FMT_COMPONENT_ALPHA: 	colour = 3; break;
	}

	int mask  = IMG_FMT_COMPONENT_RGBA |
				IMG_FMT_COMPONENT_ARGB |
				IMG_FMT_COMPONENT_BGRA |
				IMG_FMT_COMPONENT_ABGR ;

	switch( fmt & mask ) {

		default:
			assert(0);
			break;
		case IMG_FMT_COMPONENT_RGBA: order = 0; break;
		case IMG_FMT_COMPONENT_ARGB: order = 1; break;
		case IMG_FMT_COMPONENT_BGRA: order = 2; break;
		case IMG_FMT_COMPONENT_ABGR: order = 3; break;
	}

	return tbl[order][colour];
}

static float read_packed5(const struct imgData *data, int offset) {

	unsigned short s = *((unsigned short*)data->channel[0]);

	switch(offset) {
	case 0:
		s = (((s) >> 10) & 0x1f);
		break;
	case 1:
		s = (((s) >>  5) & 0x1f);
		break;
	case 2:
		s = (((s)      ) & 0x1f);
		break;
	default:
		return 0.0f;
	}

	return ((float)s) / 31.0f;
}

static void write_packed5(struct imgData *dst, int offset, float src) {

	unsigned short s =
		*(unsigned short *)dst->channel[0];

	unsigned short dsrc =
		(unsigned short)(src * 31.0f);

	static const unsigned short shifts[] = {
		10,
		 5,
		 0
	};

	static const unsigned short five_bits = 0x1f;

	static const unsigned short zero_bit = 15;

	if(offset>=3)
		return;

	s &= ~((five_bits << shifts[offset]) || (1<<zero_bit));
	s |= (dsrc & five_bits) << shifts[offset];

	(*((unsigned short*)dst->channel[0])) = s;
}

static float read_packed4(const struct imgData *data, int offset) {

	return((float)((*(((const unsigned char *)data->channel[0]) + (offset >> 1)) >> ((!(offset&1)) << 2)) & 0xff)) / 15.0f;
}

static void write_packed4(struct imgData *dst, int offset, float src) {

	unsigned char * d = (((unsigned char *)dst->channel[0]) + (offset >> 1));

	*d = (*d & (0xff                  << (( (offset&1)) << 2))) |
         (unsigned char)(src * 15.0f) << ((!(offset&1)) << 2)   ;
}

static float read_packed8(const struct imgData *data, int offset) {

	return ((float)(*(((unsigned char *)data->channel[0])+offset))) / 255.0f;
}

static void write_packed8(struct imgData *dst, int offset, float src) {

	(*(((unsigned char*)dst->channel[0]) + offset)) = (unsigned char)(src * 255.0f);
}

static float read_packed16(const struct imgData *data, int offset) {

	return ((float)(*(((unsigned short *)data->channel[0])+offset))) / 65535.0f;
}

static void write_packed16(struct imgData *dst, int offset, float src) {

	(*(((unsigned short*)dst->channel[0]) + offset)) = (unsigned char)(src * 65535.0f);
}

typedef float (*read_fnptr)  (const struct imgData*, int);
typedef void  (*write_fnptr) (struct imgData*, int, float);

static
read_fnptr get_read_function(enum imgFormat fmt) {

	if(fmt & IMG_FMT_COMPONENT_PACKED) {

		fmt &= ~IMG_FMT_COMPONENT_PACKED;

		if(fmt & (IMG_FMT_COMPONENT_PACKED24 | IMG_FMT_COMPONENT_PACKED32)) {

			return &read_packed8;
		}

		if(fmt & (IMG_FMT_COMPONENT_PACKED48 | IMG_FMT_COMPONENT_PACKED64)) {

			return &read_packed16;
		}

		if(fmt & (IMG_FMT_COMPONENT_PACKED15)) {

			return &read_packed5;
		}

		{
			// read 4-4-4-4
			int mask = (IMG_FMT_COMPONENT_PACKED16 | IMG_FMT_COMPONENT_RED | IMG_FMT_COMPONENT_GREEN | IMG_FMT_COMPONENT_BLUE | IMG_FMT_COMPONENT_ALPHA);
			if((fmt & mask) == mask) {

				return &read_packed4;
			}
		}
	}

	assert("todo:" && 0);

	return NULL;
}

static
write_fnptr get_write_function(enum imgFormat fmt) {

	if(fmt & IMG_FMT_COMPONENT_PACKED) {

		fmt &= ~IMG_FMT_COMPONENT_PACKED;

		if(fmt & (IMG_FMT_COMPONENT_PACKED24 | IMG_FMT_COMPONENT_PACKED32))
			return &write_packed8;

		if(fmt & (IMG_FMT_COMPONENT_PACKED48 | IMG_FMT_COMPONENT_PACKED64))
			return &write_packed16;

		if(fmt & (IMG_FMT_COMPONENT_PACKED15))
			return &write_packed5;

		{
			// write 4-4-4-4
			int mask = (IMG_FMT_COMPONENT_PACKED16 | IMG_FMT_COMPONENT_RED | IMG_FMT_COMPONENT_GREEN | IMG_FMT_COMPONENT_BLUE | IMG_FMT_COMPONENT_ALPHA);
			if((fmt & mask) == mask) {

				return &write_packed4;
			}
		}
	}

	assert("todo:" && 0);

	return NULL;
}

struct imgPixel imgReadPacked(const struct imgImage *img, int x, int y) {

	struct imgPixel pix = {
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};

	struct imgData imgData = imgGetPixel(img,x,y);

	read_fnptr read_px = get_read_function(img->format);

	if(img->format & IMG_FMT_COMPONENT_RED)
		pix.red 	= read_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_RED));
	if(img->format & IMG_FMT_COMPONENT_GREEN)
		pix.green 	= read_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_GREEN));
	if(img->format & IMG_FMT_COMPONENT_BLUE)
		pix.blue	= read_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_BLUE));
	if(img->format & IMG_FMT_COMPONENT_ALPHA)
		pix.alpha	= read_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_ALPHA));

	return pix;
}

void imgWritePacked(struct imgImage *img, int x, int y, struct imgPixel pix) {

	struct imgData imgData = imgGetPixel(img,x,y);

	write_fnptr write_px = get_write_function(img->format);

	if(pix.alpha == 0.0f)
		pix.red = pix.green = pix.blue = 0.0f;

	if(img->format & IMG_FMT_COMPONENT_RED)
		write_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_RED), pix.red);
	if(img->format & IMG_FMT_COMPONENT_GREEN)
		write_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_GREEN), pix.green);
	if(img->format & IMG_FMT_COMPONENT_BLUE)
		write_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_BLUE), pix.blue);
	if(img->format & IMG_FMT_COMPONENT_ALPHA)
		write_px(&imgData, packed_offset(img->format, IMG_FMT_COMPONENT_ALPHA), pix.alpha);

}

