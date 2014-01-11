
#include "internal.h"
#include<math.h>
#include<assert.h>

static float find_closest_channel_value(float channel_value, int precision_bits) {

	float i;
	float precision = (float) ((1 << precision_bits) - 1);
	if (modff(channel_value * (precision), &i) >= 0.5f)
		i += 1.0f;
	i /= (precision);

	return i;
}

static float read_pixel(struct imgImage *img, int bpp, int x, int y,
		int channel) {

	float * pixel = (float*) (((char*) img->data.channel[0]) + (x * bpp)
			+ (img->linesize[0] * y));

	return pixel[channel];
}

static void write_pixel(struct imgImage *img, int bpp, int x, int y,
		int channel, float pix) {

	float * pixel = (float*) (((char*) img->data.channel[0]) + (x * bpp)
			+ (img->linesize[0] * y));

	pixel[channel] = pix;
}

static void add_pixel(struct imgImage *img, int bpp, int x, int y, int channel,
		float pix) {

	float * pixel = (float*) (((char*) img->data.channel[0]) + (x * bpp)
			+ (img->linesize[0] * y));

	pixel[channel] += pix;
}

static int inbounds(int bx, int by, int bw, int bh, int x, int y) {

	return !(x < bx || x >= (bx + bw) || y < by || y >= (by + bh));
}

err_diffuse_kernel_t get_error_diffuse_kernel(err_diffuse_kernel_t k) {

	if (k == ERR_DIFFUSE_KERNEL_DEFAULT)
		return ERR_DIFFUSE_KERNEL_NONE;

	return k;
}

static int _error_diffuse_img(struct imgImage *img, int bx, int by, int bw,
		int bh, int bits_of_precision, int chan, err_diffuse_kernel_t edk) {

	int x, y, c, bpp;

	float _old;
	float _new;
	float _err;

	if (get_error_diffuse_kernel(edk) == ERR_DIFFUSE_KERNEL_NONE)
		return IMG_OKAY;

	bpp = imgGetBytesPerPixel(img->format, 0);

	if (bx < 0 || bx >= img->width)
		return IMG_ERROR;
	if (by < 0 || by >= img->height)
		return IMG_ERROR;
	if (bw < 0 || ((bx + bw) >= img->width))
		return IMG_ERROR;
	if (bh < 0 || ((by + bh) >= img->height))
		return IMG_ERROR;

	for (y = by; y < (by + bh); y++) {

		if (y < 0 || y >= img->height)
			return IMG_ERROR;

		for (x = bx; x < (bx + bw); x++) {

			if (x < 0 || x >= img->width)
				return IMG_ERROR;

			for (c = 0; c < chan; c++) {

				switch (get_error_diffuse_kernel(edk)) {

				case ERR_DIFFUSE_KERNEL_NONE:
				case ERR_DIFFUSE_KERNEL_DEFAULT:
					break; // SHUTUP GCC - 'edk' is NEVER NONE or DEFAULT.

				case ERR_DIFFUSE_KERNEL_SMALLEST: /* my experimentations (minimal) */
					_old = read_pixel(img, bpp, x, y, c);
					_new = find_closest_channel_value(_old, bits_of_precision);
					write_pixel(img, bpp, x, y, c, _new);
					_err = _old - _new;
					if (inbounds(bx, by, bw, bh, x + 1, y))
						add_pixel(img, bpp, x + 1, y, c, (3.0f / 8.0f) * _err);
					if (inbounds(bx, by, bw, bh, x, y + 1))
						add_pixel(img, bpp, x, y + 1, c, (3.0f / 8.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 1, y + 1))
						add_pixel(img, bpp, x + 1, y + 1, c,
								(2.0f / 8.0f) * _err);
					break;

				case ERR_DIFFUSE_KERNEL_MEDIUM: /* FLOYD STEINBERG */
					_old = read_pixel(img, bpp, x, y, c);
					_new = find_closest_channel_value(_old, bits_of_precision);
					write_pixel(img, bpp, x, y, c, _new);
					_err = _old - _new;
					if (inbounds(bx, by, bw, bh, x + 1, y))
						add_pixel(img, bpp, x + 1, y, c, (7.0f / 16.0f) * _err);
					if (inbounds(bx, by, bw, bh, x - 1, y + 1))
						add_pixel(img, bpp, x - 1, y + 1, c,
								(3.0f / 16.0f) * _err);
					if (inbounds(bx, by, bw, bh, x, y + 1))
						add_pixel(img, bpp, x, y + 1, c, (5.0f / 16.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 1, y + 1))
						add_pixel(img, bpp, x + 1, y + 1, c,
								(1.0f / 16.0f) * _err);
					break;

				case ERR_DIFFUSE_KERNEL_LARGEST: /* J F Jarvis, C N Judice, and W H Ninke */
					_old = read_pixel(img, bpp, x, y, c);
					_new = find_closest_channel_value(_old, bits_of_precision);
					write_pixel(img, bpp, x, y, c, _new);
					_err = _old - _new;
					if (inbounds(bx, by, bw, bh, x + 1, y))
						add_pixel(img, bpp, x + 1, y, c, (7.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 2, y))
						add_pixel(img, bpp, x + 2, y, c, (5.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x - 2, y + 1))
						add_pixel(img, bpp, x - 2, y + 1, c,
								(3.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x - 1, y + 1))
						add_pixel(img, bpp, x - 1, y + 1, c,
								(5.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x, y + 1))
						add_pixel(img, bpp, x, y + 1, c, (7.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 1, y + 1))
						add_pixel(img, bpp, x + 1, y + 1, c,
								(5.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 2, y + 1))
						add_pixel(img, bpp, x + 2, y + 1, c,
								(3.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x - 2, y + 2))
						add_pixel(img, bpp, x - 2, y + 2, c,
								(1.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x - 1, y + 2))
						add_pixel(img, bpp, x - 1, y + 2, c,
								(3.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x, y + 2))
						add_pixel(img, bpp, x, y + 2, c, (5.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 1, y + 2))
						add_pixel(img, bpp, x + 1, y + 2, c,
								(3.0f / 48.0f) * _err);
					if (inbounds(bx, by, bw, bh, x + 2, y + 2))
						add_pixel(img, bpp, x + 2, y + 2, c,
								(1.0f / 48.0f) * _err);
					break;
				}
			}
		}
	}

	/*** CLAMP to 0 .. 1 ***/
	for (y = 0; y < img->height; y++) {

		float * pixel = (float*) (((char*) img->data.channel[0])
				+ (img->linesize[0] * y));
		for (x = 0; x < img->width; x++) {

			for (c = 0; c < chan; c++) {

				const float v = *pixel;
				const float b = v < 0.0f ? 0.0f : v;
				*pixel = b > 1.0f ? 1.0f : b;

				pixel++;
			}
		}
	}

	return IMG_OKAY;
}

int imguErrorDiffuseArea(struct imgImage *img, int x, int y, int w, int h,
		int bits_of_precision, err_diffuse_kernel_t edk) {

	assert(img->format & IMG_FMT_COMPONENT_FLOAT);

	return _error_diffuse_img(img, x, y, w, h, bits_of_precision,
			imgGetBytesPerPixel(img->format, 0) / sizeof(float), edk);
}

int imguErrorDiffuse(struct imgImage *img, int bits_of_precision,
		err_diffuse_kernel_t edk) {

	return imguErrorDiffuseArea(img, 0, 0, img->width, img->height,
			bits_of_precision, edk);
}

