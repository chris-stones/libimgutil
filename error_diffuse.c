

#include "internal.h"
#include<math.h>
#include<assert.h>

static float find_closest_channel_value(float channel_value, int precision_bits ) {

	float i;
	float precision = (float)((1 << precision_bits)-1);
	if( modff( channel_value * (precision), &i ) >= 0.5f )
		i += 1.0f;
	i /= (precision);

	return i;
}

static float read_pixel(struct imgImage *img, int bpp, int x, int y, int channel) {

	float * pixel = (float*)(((char*)img->data.channel[0]) + (x * bpp) + (img->linesize[0] * y));

	return pixel[channel];
}

static void write_pixel(struct imgImage *img, int bpp, int x, int y, int channel, float pix) {

	float * pixel = (float*)(((char*)img->data.channel[0]) + (x * bpp) + (img->linesize[0] * y));

	pixel[channel] = pix;
}

static void add_pixel(struct imgImage *img, int bpp, int x, int y, int channel, float pix) {

	float * pixel = (float*)(((char*)img->data.channel[0]) + (x * bpp) + (img->linesize[0] * y));

	pixel[channel] += pix;
}

static int inbounds(struct imgImage *img, int x, int y) {

	return !( x<0 || x>=img->width || y<0 || y>=img->height );
}

void _error_diffuse_img( struct imgImage *img, int bits_of_precision, int chan ) {

	int x,y,c,bpp;

	float _old;
	float _new;
	float _err;

	bpp = imgGetBytesPerPixel(img->format,0);

	for(y=0; y<img->height; y++) {

		for(x=0; x<img->width; x++) {

			for(c=0;c<chan;c++) {

				_old = read_pixel(img,bpp,x,y,c);
				_new = find_closest_channel_value(_old, bits_of_precision);
				write_pixel(img,bpp,x,y,c,_new);
				_err = _old - _new;

				/* my experimentations */
				if(inbounds(img,x+1,y  )) add_pixel( img, bpp, x+1, y  , c, (3.0f/8.0f) * _err );
				if(inbounds(img,x  ,y+1)) add_pixel( img, bpp, x  , y+1, c, (3.0f/8.0f) * _err );
				if(inbounds(img,x+1,y+1)) add_pixel( img, bpp, x+1, y+1, c, (2.0f/8.0f) * _err );

				/* FLOYD STEINBERG */
//				if(inbounds(img,x+1,y  )) add_pixel( img, bpp, x+1, y  , c, (7.0f/16.0f) * _err );
//				if(inbounds(img,x-1,y+1)) add_pixel( img, bpp, x-1, y+1, c, (3.0f/16.0f) * _err );
//				if(inbounds(img,x  ,y+1)) add_pixel( img, bpp, x  , y+1, c, (5.0f/16.0f) * _err );
//				if(inbounds(img,x+1,y+1)) add_pixel( img, bpp, x+1, y+1, c, (1.0f/16.0f) * _err );

				/* J F Jarvis, C N Judice, and W H Ninke */
//				if(inbounds(img,x+1,y  )) add_pixel( img, bpp, x+1, y  , c, (7.0f/48.0f) * _err );
//				if(inbounds(img,x+2,y  )) add_pixel( img, bpp, x+2, y  , c, (5.0f/48.0f) * _err );
//				if(inbounds(img,x-2,y+1)) add_pixel( img, bpp, x-2, y+1, c, (3.0f/48.0f) * _err );
//				if(inbounds(img,x-1,y+1)) add_pixel( img, bpp, x-1, y+1, c, (5.0f/48.0f) * _err );
//				if(inbounds(img,x  ,y+1)) add_pixel( img, bpp, x  , y+1, c, (7.0f/48.0f) * _err );
//				if(inbounds(img,x+1,y+1)) add_pixel( img, bpp, x+1, y+1, c, (5.0f/48.0f) * _err );
//				if(inbounds(img,x+2,y+1)) add_pixel( img, bpp, x+2, y+1, c, (3.0f/48.0f) * _err );
//				if(inbounds(img,x-2,y+2)) add_pixel( img, bpp, x-2, y+2, c, (1.0f/48.0f) * _err );
//				if(inbounds(img,x-1,y+2)) add_pixel( img, bpp, x-1, y+2, c, (3.0f/48.0f) * _err );
//				if(inbounds(img,x  ,y+2)) add_pixel( img, bpp, x  , y+2, c, (5.0f/48.0f) * _err );
//				if(inbounds(img,x+1,y+2)) add_pixel( img, bpp, x+1, y+2, c, (3.0f/48.0f) * _err );
//				if(inbounds(img,x+2,y+2)) add_pixel( img, bpp, x+2, y+2, c, (1.0f/48.0f) * _err );
			}
		}
	}

	/*** CLAMP ***/
	for(y=0; y<img->height; y++) {

		float * pixel = (float*)(((char*)img->data.channel[0]) + (img->linesize[0] * y));
		for(x=0; x<img->width; x++) {

			for(c=0;c<chan;c++) {

				const float v = *pixel;
				const float b = v < 0.0f ? 0.0f : v;
				*pixel =        b > 1.0f ? 1.0f : b;

				pixel++;
			}
		}
	}
}

void error_diffuse_img( struct imgImage *img, int bits_of_precision ) {

	assert(img->format & IMG_FMT_COMPONENT_FLOAT);

	_error_diffuse_img(img, bits_of_precision, imgGetBytesPerPixel(img->format,0) / sizeof(float) );
}

