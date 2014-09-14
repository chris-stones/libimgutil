
/*
 * compressed_pixel_ops.c
 *
 *  Created on: Feb 23, 2013
 *      Author: cds
 */

#include "internal.h"

#if !defined(WIN32)
#include <unistd.h>
#endif

/***
 * <txc_dxtn.h> used for texture decompression.
 * http://cgit.freedesktop.org/~mareko/libtxc_dxtn/
 * on Gentoo - "emerge media-libs/libtxc_dxtn"
 */
#include<txc_dxtn.h>

/***
 * <csquish.h> used for texture compression.
 * https://github.com/chris-stones/libcsquish.git
 */
#include<csquish.h>

/***
 * rg_etc1 used for texture compression / decompression
 * https://code.google.com/p/rg-etc1/
 ***/
#include "3rdparty/rg_etc1.h"

#include<assert.h>
#include <pthread.h>

struct imgPixel imgReadCompressed(const struct imgImage *img, int x, int y) {

	unsigned char decompressed[4] = { 255, 0, 255, 255 }; // RGBA

	struct imgPixel pix = { 0.0f, 0.0f, 0.0f, 1.0f };

	if( ( img->format & IMG_FMT_COMPONENT_COMPRESSION_INDEX_MASK ) == IMG_FMT_COMPONENT_ETC1_INDEX ) {

		assert( 0 && "ETC1 DECOMPRESS NOT YET IMPLEMENTED!" );
		return pix;
	}

	switch (img->format & IMG_FMT_COMPONENT_COMPRESSION_INDEX_MASK) {
	case IMG_FMT_COMPONENT_DXT1_INDEX: // no alpha

		fetch_2d_texel_rgb_dxt1(img->linesize[0], img->data.channel[0], x, y,
				decompressed);
		break;

	case IMG_FMT_COMPONENT_DXT3_INDEX: // sharp alpha

		fetch_2d_texel_rgba_dxt3(img->linesize[0], img->data.channel[0], x, y,
				decompressed);
		break;

	case IMG_FMT_COMPONENT_DXT5_INDEX: // gradient alpha

		fetch_2d_texel_rgba_dxt5(img->linesize[0], img->data.channel[0], x, y,
				decompressed);
		break;
	}

	pix.red = ((float) decompressed[0]) / 255.0f;
	pix.green = ((float) decompressed[1]) / 255.0f;
	pix.blue = ((float) decompressed[2]) / 255.0f;
	pix.alpha = ((float) decompressed[3]) / 255.0f;

	return pix;
}

static int neet_to_reformat(const struct imgImage *dst,
		const struct imgImage *src) {

	if (need_to_pma(dst, src))
		return 1;
	if (need_to_unpma(dst, src))
		return 1;
	if (dst->format & IMG_FMT_COMPONENT_ALPHA)
		return (src->format & IMG_FMT_RGBA32) != IMG_FMT_RGBA32;
	return (src->format & IMG_FMT_RGB24) != IMG_FMT_RGB24;
}

static int glFormatToCSquishFormat(GLenum glFmt) {

	switch (glFmt) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		return SQUISH_kDxt1 | SQUISH_kColourIterativeClusterFit;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		return SQUISH_kDxt3 | SQUISH_kColourIterativeClusterFit;
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		return SQUISH_kDxt5 | SQUISH_kColourIterativeClusterFit;
	}
	assert(0);
	return 0; // quiet GCC
}

struct job_struct {

	unsigned char const* rgba;
	void * blocks;
	int w;
	int h;
	int flags;
	float * metric;
};

static void* thread_func(void * arg) {

	struct job_struct * job = (struct job_struct *) arg;

	squish_CompressImage(job->rgba, job->w, job->h, job->blocks, job->flags,
			job->metric);

	free(job);

	return NULL;
}

void squish_CompressImage_mt(unsigned char const* rgba, int width, int height,
		void* blocks, int flags, float* metric) {

	// TODO: implement threading on win32.

	int number_of_threads = 1;

	if (width * height > 4096) {

#if defined(WIN32)
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		number_of_threads = sysinfo.dwNumberOfProcessors;
#else
		number_of_threads = sysconf( _SC_NPROCESSORS_ONLN);
#endif

		if (number_of_threads < 1)
			number_of_threads = 4; // can't determine, assume 4 threads.

		if (number_of_threads > ((height + 3) / 4))
			number_of_threads = (height + 3) / 4;

		if (number_of_threads < 1)
			number_of_threads = 1;
	}

	if (number_of_threads > 1) {
		int thread;
		int h = 0;
		int y = 0;
		int hblocks = ((((height + 3) / 4) + number_of_threads)
				/ number_of_threads);

#ifndef WIN32
		pthread_t * pthreads = alloca(sizeof(pthread_t) * number_of_threads);
#endif

		for (thread = 0; thread < number_of_threads; thread++) {

			struct job_struct * job = malloc(sizeof(struct job_struct));

			h = 4 * hblocks;
			if ((y + h) > height)
				h = height - y;

			job->w = width;
			job->h = h;
			job->flags = flags;
			job->metric = metric;
			job->rgba = rgba + job->w * y * 4;

			if (flags & SQUISH_kDxt1)
				job->blocks = ((char*) blocks)
						+ (((width + 3) / 4) * ((y + 3) / 4) * 8);
			else
				job->blocks = ((char*) blocks)
						+ (((width + 3) / 4) * ((y + 3) / 4) * 16);

			y += h;
#ifndef WIN32
			pthread_create(pthreads + thread, NULL, &thread_func, job);
#else
			thread_func(job);
#endif
		}
#ifndef WIN32
		for (; thread > 0; thread--)
			pthread_join(pthreads[thread - 1], NULL);
#endif
	} else
		squish_CompressImage(rgba, width, height, blocks, flags, metric);
}

void imgWriteCompressed(struct imgImage *dst, const struct imgImage *csrc) {

	// TODO: unlike other pixel operations, this can fail. ENOMEM etc. HANDLE IT

	int err;
	/////////  UGLY! /////////////////////////////////
	// src is a pointer to const src parameter
	// or a temporary variable copy in a more suitable format.
	struct imgImage vsrc = *csrc;
	struct imgImage *_src = &vsrc;
	struct imgImage *src = _src;
	//////////////////////////////////////////////////
	GLenum gl_fmt;

	if ((dst->format & IMG_FMT_COMPONENT_COMPRESSION_INDEX_MASK) == IMG_FMT_COMPONENT_ETC1_INDEX) {

//		gl_fmt = 0x8D64; /* ETC1_RGB8_OES; ( name string GL_OES_compressed_ETC1_RGB8_texture ) */

		if ((src->format != IMG_FMT_RGBA32) || (src->width % 4) || (src->height % 4)) {

			src = NULL;
			err = imgAllocImage(&src);
			assert(err == IMG_OKAY);
			src->format = IMG_FMT_RGBA32;
			src->width = _src->width;
			src->height = _src->height;
			if(src->width % 4)
				src->width += 4 - (src->width % 4);
			if(src->height % 4)
				src->height += 4 - (src->height % 4);

			err = imgAllocPixelBuffers(src);
			assert(err == IMG_OKAY);
			err = imguCopyImage(src, _src);
		}

		rg_etc1_pack_etc1_block_init();

		{
			// TODO: this needs threading!
			char * dst_data_etc1 = (char *)dst->data.channel[0];
			const int * src_data_rgba32 = (const int *)src->data.channel[0];
			int srcblock[16];
			int x,y;
			struct rg_etc1_etc1_pack_params etc1_params;
			etc1_params.m_dithering = 1;
			etc1_params.m_quality = cHighQuality;

			for(y=0;y<src->height;y+=4)
				for(x=0;x<src->width;x+=4) {

					srcblock[ 0] = src_data_rgba32[x + 0 + ((y+0) * src->width)];
					srcblock[ 1] = src_data_rgba32[x + 1 + ((y+0) * src->width)];
					srcblock[ 2] = src_data_rgba32[x + 2 + ((y+0) * src->width)];
					srcblock[ 3] = src_data_rgba32[x + 3 + ((y+0) * src->width)];
					srcblock[ 4] = src_data_rgba32[x + 0 + ((y+1) * src->width)];
					srcblock[ 5] = src_data_rgba32[x + 1 + ((y+1) * src->width)];
					srcblock[ 6] = src_data_rgba32[x + 2 + ((y+1) * src->width)];
					srcblock[ 7] = src_data_rgba32[x + 3 + ((y+1) * src->width)];
					srcblock[ 8] = src_data_rgba32[x + 0 + ((y+2) * src->width)];
					srcblock[ 9] = src_data_rgba32[x + 1 + ((y+2) * src->width)];
					srcblock[10] = src_data_rgba32[x + 2 + ((y+2) * src->width)];
					srcblock[11] = src_data_rgba32[x + 3 + ((y+2) * src->width)];
					srcblock[12] = src_data_rgba32[x + 0 + ((y+3) * src->width)];
					srcblock[13] = src_data_rgba32[x + 1 + ((y+3) * src->width)];
					srcblock[14] = src_data_rgba32[x + 2 + ((y+3) * src->width)];
					srcblock[15] = src_data_rgba32[x + 3 + ((y+3) * src->width)];

					rg_etc1_pack_etc1_block(dst_data_etc1,srcblock,&etc1_params);
					dst_data_etc1 += 8;
				}
		}
	}
	else if ((dst->format & IMG_FMT_COMPONENT_COMPRESSION_INDEX_MASK) == IMG_FMT_COMPONENT_DXT1_INDEX) {

		const GLenum gl_fmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

		if (src->format != IMG_FMT_RGBA32) {

			src = NULL;
			err = imgAllocImage(&src);
			assert(err == IMG_OKAY);
			src->format = IMG_FMT_RGBA32;
			src->width = _src->width;
			src->height = _src->height;
			err = imgAllocPixelBuffers(src);
			assert(err == IMG_OKAY);
			err = imguCopyImage(src, _src);
		}

		squish_CompressImage_mt(src->data.channel[0], src->width, src->height,
			dst->data.channel[0], glFormatToCSquishFormat(gl_fmt), NULL);

	} else {

		GLenum gl_fmt;

		switch (dst->format & IMG_FMT_COMPONENT_COMPRESSION_INDEX_MASK) {
		case IMG_FMT_COMPONENT_DXT3_INDEX:
			gl_fmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case IMG_FMT_COMPONENT_DXT5_INDEX:
			gl_fmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			assert(0);
		}

		if (neet_to_reformat(dst, src)) {

			src = NULL;
			err = imgAllocImage(&src);
			assert(err == IMG_OKAY);

			src->format = IMG_FMT_RGBA32;
			if (dst->format & IMG_FMT_COMPONENT_PMA)
				src->format |= IMG_FMT_COMPONENT_PMA;

			src->width = _src->width;
			src->height = _src->height;
			err = imgAllocPixelBuffers(src);
			assert(err == IMG_OKAY);
			assert((src->format & IMG_FMT_COMPONENT_COMPRESSED) == 0); // imguCopyImage uses this function to handle compressed textures. avoid infinite recursion!
			err = imguCopyImage(src, _src);
		}

		squish_CompressImage_mt(src->data.channel[0], src->width, src->height,
			dst->data.channel[0], glFormatToCSquishFormat(gl_fmt), NULL);
	}

	if (src != _src) {
		imgFreeAll(src);
	}

}

