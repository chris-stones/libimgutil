/*
 * libimgutil.h
 *
 *  Created on: Oct 29, 2011
 *      Author: cds
 */

#ifndef LIBIMGUTIL_H_
#define LIBIMGUTIL_H_

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

int imguCopyImage(struct imgImage *dst, const struct imgImage *src);
int imguCopyImage2(struct imgImage *dst, const struct imgImage *src,
		err_diffuse_kernel_t edk);
int imguCopyImage3(struct imgImage *dst, const struct imgImage *src,
		err_diffuse_kernel_t edk, copy_quality_t quality);

int imguCopyPixel(struct imgImage *dst, const struct imgImage *src, int dx,
		int dy, int sx, int sy);
int imguCopyRect(struct imgImage *dst, const struct imgImage *src, int dx,
		int dy, int sx, int sy, int w, int h);
int imguCopyRect2(struct imgImage *dst, const struct imgImage *src, int dx,
		int dy, int sx, int sy, int w, int h, err_diffuse_kernel_t edk);

int imguRotateCW(struct imgImage **rotated, const struct imgImage *src);
int imguPad(struct imgImage **padded, const struct imgImage *src, int top,
		int bot, int left, int right);

int imguBinaryCompare(struct imgImage *a, struct imgImage *b);
int imguBinaryHash32(struct imgImage *a);

int imguErrorDiffuseArea(struct imgImage *img, int x, int y, int w, int h,
		int bits_of_precision, err_diffuse_kernel_t edk);
int imguErrorDiffuse(struct imgImage *img, int bits_of_precision,
		err_diffuse_kernel_t edk);

enum imgFormat imguGetFormatByName(const char * name);
const char * imguGetFormatName(enum imgFormat fmt);

#ifdef __cplusplus
} /*** extern "C" { ***/
#endif

#endif /* LIBIMGUTIL_H_ */
