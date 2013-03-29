/*
 * planar_pixel_ops.c
 *
 *  Created on: Oct 30, 2011
 *      Author: cds
 */

#include "internal.h"

#include<assert.h>


struct mat3x3 {

	float f[3][3];
};

struct mat3x1 {
	float f[3][1];
};

static const struct mat3x3 rgb2ycbcr_mat = {

	{
		{ 0.299f, 0.587f, 0.114f},
		{-0.169f,-0.331f, 0.500f},
		{ 0.500f,-0.419f,-0.081f}
	}
};

static const struct mat3x3 ycbcr2rgb_mat = {

	{
		{ 1.000f, 0.000f, 1.400f},
		{ 1.000f,-0.343f,-0.711f},
		{ 1.000f, 1.765f, 0.000f},
	}
};

static void matmult(const struct mat3x3 *m33, const struct mat3x1 *m31, struct mat3x1 *out) {

	float m33_00 = m33->f[0][0];
	float m33_10 = m33->f[1][0];
	float m33_20 = m33->f[2][0];
	float m33_01 = m33->f[0][1];
	float m33_11 = m33->f[1][1];
	float m33_21 = m33->f[2][1];
	float m33_02 = m33->f[0][2];
	float m33_12 = m33->f[1][2];
	float m33_22 = m33->f[2][2];
	float m31_00 = m31->f[0][0];
	float m31_10 = m31->f[1][0];
	float m31_20 = m31->f[2][0];


	out->f[0][0] = m33_00 * m31_00 + m33_01 * m31_10 + m33_02 * m31_20;
	out->f[1][0] = m33_10 * m31_00 + m33_11 * m31_10 + m33_12 * m31_20;
	out->f[2][0] = m33_20 * m31_00 + m33_21 * m31_10 + m33_22 * m31_20;
}

static float clampf(float f) {

	if(f<0.0f)
		return 0.0f;
	if(f>1.0f)
		return 1.0f;
	return f;
}

static void rgb2ycbcr(struct imgPixel *px, float *y, float *cb, float *cr) {

	struct mat3x1 rgb,ycbcr;

	rgb.f[0][0] = px->red;
	rgb.f[1][0] = px->green;
	rgb.f[2][0] = px->blue;

	matmult(&rgb2ycbcr_mat, &rgb, &ycbcr);

	ycbcr.f[1][0] += 0.5f;
	ycbcr.f[2][0] += 0.5f;

	 *y = clampf(ycbcr.f[0][0]);
	*cb = clampf(ycbcr.f[1][0]);
	*cr = clampf(ycbcr.f[2][0]);
}

static void ycbcr2rgb(float y, float cb, float cr, struct imgPixel *px) {

	struct mat3x1 ycbcr,rgb;

	ycbcr.f[0][0] = y;
	ycbcr.f[1][0] = cb - 0.5f;
	ycbcr.f[2][0] = cr - 0.5f;

	matmult(&ycbcr2rgb_mat, &ycbcr, &rgb);

	px->red 	= clampf(rgb.f[0][0]);
	px->green 	= clampf(rgb.f[1][0]);
	px->blue 	= clampf(rgb.f[2][0]);
}

static int planar_channel(enum imgFormat fmt, int component_bit) {

	static const int tbl[4][4] = {
		// Y  CB  CR  A
		{  0,  1,  2, 3 }, // YCBCRA
		{  0,  2,  1, 3 }, // YCRCBA
	};

	int order=0;
	int colour=0;

	switch(component_bit) {
		case IMG_FMT_COMPONENT_Y: 		colour = 0; break;
		case IMG_FMT_COMPONENT_CB:		colour = 1; break;
		case IMG_FMT_COMPONENT_CR: 		colour = 2; break;
		case IMG_FMT_COMPONENT_ALPHA: 	colour = 3; break;
	}

	int mask  = IMG_FMT_COMPONENT_YCBCRA |
				IMG_FMT_COMPONENT_YCRCBA ;

	switch( fmt & mask ) {

		default:
			assert(0);
			break;
		case IMG_FMT_COMPONENT_YCBCRA: order = 0; break;
		case IMG_FMT_COMPONENT_YCRCBA: order = 1; break;
	}

	return tbl[order][colour];
}

static float read_planar8(const struct imgData *data, int channel) {

	return ((float)(*(((unsigned char *)data->channel[channel])+0))) / 255.0f;
}

static void write_planar8(struct imgData *dst, int channel, float src) {

	(*(((unsigned char*)dst->channel[channel]) + 0)) = (unsigned char)(src * 255.0f);
}

typedef float (*read_fnptr)  (const struct imgData*, int);
typedef void (*write_fnptr) (struct imgData*, int, float);

static
read_fnptr get_read_function(enum imgFormat fmt) {

	if(fmt & IMG_FMT_COMPONENT_420P)
		return &read_planar8;

	assert("todo:" && 0);

	return NULL;
}

static
write_fnptr get_write_function(enum imgFormat fmt) {

	if(fmt & IMG_FMT_COMPONENT_420P)
		return &write_planar8;

	assert("todo:" && 0);

	return NULL;
}

struct imgPixel imgReadPlanar(const struct imgImage *img, int x, int y) {

	struct imgPixel pix = {
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};

	float Y,Cb,Cr;

	struct imgData imgData = imgGetPixel(img,x,y);

	read_fnptr read_px = get_read_function(img->format);

	if(img->format & IMG_FMT_COMPONENT_Y)
		Y = read_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_Y));
	if(img->format & IMG_FMT_COMPONENT_CB)
		Cb = read_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_CB));
	if(img->format & IMG_FMT_COMPONENT_CR)
		Cr = read_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_CR));
	if(img->format & IMG_FMT_COMPONENT_ALPHA)
		pix.alpha	= read_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_ALPHA));

	ycbcr2rgb(Y,Cb,Cr, &pix);

	return pix;
}

void imgWritePlanar(struct imgImage *img, int x, int y, struct imgPixel pix) {

	float Y,Cb,Cr;
	struct imgData imgData = imgGetPixel(img,x,y);

	write_fnptr write_px = get_write_function(img->format);

	rgb2ycbcr(&pix, &Y, &Cb, &Cr);

	if(pix.alpha == 0.0f)
		Y = 0.0f;

	if(img->format & IMG_FMT_COMPONENT_Y)
		write_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_Y), Y);
	if(img->format & IMG_FMT_COMPONENT_CB)
		write_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_CB), Cb);
	if(img->format & IMG_FMT_COMPONENT_CR)
		write_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_CR), Cr);
	if(img->format & IMG_FMT_COMPONENT_ALPHA)
		write_px(&imgData, planar_channel(img->format, IMG_FMT_COMPONENT_ALPHA), pix.alpha);
}



