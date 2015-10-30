/*
 * libimgutil.h
 *
 *  Created on: Oct 29, 2011
 *      Author: cds
 */

#pragma once

#if defined(_MSC_VER)
  #if defined(LIBIMGUTIL_EXPORTS)
    #define LIBIMGUTIL_DLL __declspec(dllexport)
  #else
    #define LIBIMGUTIL_DLL __declspec(dllimport)
  #endif
#else
  #define LIBIMGUTIL_DLL
#endif

/***
 * https://github.com/chris-stones/libimg
 */
#include<libimg.h>

#define IMGU_OKAY IMG_OKAY
#define IMGU_ERROR -1
#define IMGU_OUT_OF_BOUNDS -2

/*
 smaller kernels hide colour banding better,
 but larger kernels compress better.
 */
typedef enum {

	ERR_DIFFUSE_KERNEL_NONE = 0,
	ERR_DIFFUSE_KERNEL_DEFAULT = 1,
	ERR_DIFFUSE_KERNEL_SMALLEST = 100,
	ERR_DIFFUSE_KERNEL_MEDIUM = 10000,
	ERR_DIFFUSE_KERNEL_LARGEST = 1000000,

} err_diffuse_kernel_t;

/*
 * quality settings.
 * presently used only in texture compression.
 */
typedef enum {

	COPY_QUALITY_NONE = 0,
	COPY_QUALITY_DEFAULT = 1,
	COPY_QUALITY_LOWEST = 100,
	COPY_QUALITY_MEDIUM = 10000,
	COPY_QUALITY_HIGHEST = 1000000,

} copy_quality_t;

#ifdef __cplusplus
extern "C" {
#endif

int LIBIMGUTIL_DLL imguCopyImage(struct imgImage *dst, const struct imgImage *src);
int LIBIMGUTIL_DLL imguCopyImage2(struct imgImage *dst, const struct imgImage *src,
		err_diffuse_kernel_t edk);
int LIBIMGUTIL_DLL imguCopyImage3(struct imgImage *dst, const struct imgImage *src,
		err_diffuse_kernel_t edk, copy_quality_t quality);

int LIBIMGUTIL_DLL imguCopyPixel(struct imgImage *dst, const struct imgImage *src, int dx,
		int dy, int sx, int sy);
int LIBIMGUTIL_DLL imguCopyRect(struct imgImage *dst, const struct imgImage *src, int dx,
		int dy, int sx, int sy, int w, int h);
int LIBIMGUTIL_DLL imguCopyRect2(struct imgImage *dst, const struct imgImage *src, int dx,
		int dy, int sx, int sy, int w, int h, err_diffuse_kernel_t edk);

int LIBIMGUTIL_DLL imguRotateCW(struct imgImage **rotated, const struct imgImage *src);
int LIBIMGUTIL_DLL imguPad(struct imgImage **padded, const struct imgImage *src, int top,
		int bot, int left, int right);

int LIBIMGUTIL_DLL imguBinaryCompare(struct imgImage *a, struct imgImage *b);
int LIBIMGUTIL_DLL imguBinaryHash32(struct imgImage *a);

int LIBIMGUTIL_DLL imguErrorDiffuseArea(struct imgImage *img, int x, int y, int w, int h,
		int bits_of_precision, err_diffuse_kernel_t edk);
int LIBIMGUTIL_DLL imguErrorDiffuse(struct imgImage *img, int bits_of_precision,
		err_diffuse_kernel_t edk);

enum imgFormat LIBIMGUTIL_DLL imguGetFormatByName(const char * name);
const char LIBIMGUTIL_DLL * imguGetFormatName(enum imgFormat fmt);

#ifdef __cplusplus
} /*** extern "C" { ***/
#endif


