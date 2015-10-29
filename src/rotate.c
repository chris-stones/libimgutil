#include "internal.h"
#include <assert.h>
#include <string.h>

int LIBIMGUTIL_DLL imguRotateCW(struct imgImage **rotated, const struct imgImage *src) {

	struct imgImage *out = NULL;

	if( imgAllocImage( &out ) != IMG_OKAY )
	return -1;

	out->format = src->format;
	out->width = src->height;
	out->height = src->width;

	if( imgAllocPixelBuffers( out ) != IMG_OKAY ) {

		imgFreeAll(out);
		return -1;
	}

	assert( !(src->format & IMG_FMT_COMPONENT_COMPRESSED) && "TODO: impliment compressed rotation" );
	assert( !(src->format & IMG_FMT_COMPONENT_PLANAR) && "TODO: impliment planar rotation" );

	{
		int sx,sy,dx,dy,channels,channel;
		struct imgData src_data;
		struct imgData dst_data;

		channels = imgGetChannels(src->format);

		for(sx=0;sx<src->width;sx++)
		for(sy=0;sy<src->height;sy++) {

			dx = src->height - (sy+1);
			dy = sx;

			src_data = imgGetPixel( src, sx, sy );
			dst_data = imgGetPixel( out, dx, dy );

			for(channel=0;channel<channels;channel++)
			memcpy( dst_data.channel[channel], src_data.channel[channel], imgGetBytesPerPixel(src->format, channel) );
		}
	}

	*rotated = out;

	return IMG_OKAY;
}

