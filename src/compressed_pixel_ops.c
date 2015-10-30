
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
//#include<txc_dxtn.h>

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

#include <assert.h>

struct imgPixel imgReadCompressed(const struct imgImage *img, int x, int y) {

	assert( 0 && "PIXEL DECOMPRESS NOT YET IMPLEMENTED!" );

	struct imgPixel pix = { 0.0f, 0.0f, 0.0f, 1.0f };
	/*
	unsigned char decompressed[4] = { 255, 0, 255, 255 }; // RGBA

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

*/
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

static int glFormatToCSquishFormat(GLenum glFmt, copy_quality_t quality) {

	int qualityFlags;

	switch(quality) {
	default:
	case COPY_QUALITY_HIGHEST:
		qualityFlags = SQUISH_kColourIterativeClusterFit;
		break;
	case COPY_QUALITY_MEDIUM:
		qualityFlags = SQUISH_kColourClusterFit;
		break;
	case COPY_QUALITY_LOWEST:
		qualityFlags = SQUISH_kColourRangeFit;
		break;
	}

	switch (glFmt) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		return SQUISH_kDxt1  | qualityFlags;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		return SQUISH_kDxt3 | qualityFlags;
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		return SQUISH_kDxt5 | qualityFlags;
	}
	assert(0);
	return 0; // quiet GCC
}

int imgWriteCompressed(struct imgImage *dst, const struct imgImage *csrc, copy_quality_t quality) {

	// TODO: unlike other pixel operations, this can fail. ENOMEM etc. HANDLE IT

	if(quality == COPY_QUALITY_DEFAULT || quality == COPY_QUALITY_NONE)
		quality = COPY_QUALITY_HIGHEST;

	int err;
	/////////  UGLY! /////////////////////////////////
	// src is a pointer to const src parameter
	// or a temporary variable copy in a more suitable format.
	struct imgImage vsrc = *csrc;
	struct imgImage *_src = &vsrc;
	struct imgImage *src = _src;
	//////////////////////////////////////////////////

	if(dst->width != csrc->width || dst->height != csrc->height)
		return -1; // src and dst must be same size.

	if(dst->width % 4 || dst->height % 4)
		return -1; // sizes must be multiples of 4.

	if ((dst->format & IMG_FMT_COMPONENT_COMPRESSION_INDEX_MASK) == IMG_FMT_COMPONENT_ETC1_INDEX) {

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

		rg_etc1_pack_etc1_block_init();

		{
			// TODO: this needs threading!
			char * dst_data_etc1 = (char *)dst->data.channel[0];
			const int * src_data_rgba32 = (const int *)src->data.channel[0];
			unsigned int srcblock[16];
			int x,y,a;
			struct rg_etc1_etc1_pack_params etc1_params;
			etc1_params.m_dithering = 1;

			switch(quality) {
			default:
			case COPY_QUALITY_HIGHEST:
				etc1_params.m_quality = cHighQuality;
				break;
			case COPY_QUALITY_MEDIUM:
				etc1_params.m_quality = cMediumQuality;
				break;
			case COPY_QUALITY_LOWEST:
				etc1_params.m_quality = cLowQuality;
				break;
			}

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

					// Set alpha channel to 0xFF
					for (a = 3; a < sizeof(srcblock); a += 4)
						*(((unsigned char*)(srcblock)) + a) = 0xff;

					rg_etc1_pack_etc1_block(dst_data_etc1,(const unsigned int*)srcblock,&etc1_params);
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

		squish_CompressImage(src->data.channel[0], src->width, src->height,
			dst->data.channel[0], glFormatToCSquishFormat(gl_fmt, quality), NULL);

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

		squish_CompressImage(src->data.channel[0], src->width, src->height,
			dst->data.channel[0], glFormatToCSquishFormat(gl_fmt, quality), NULL);
	}

	if (src != _src) {
		imgFreeAll(src);
	}

	return 0;
}

