#include "internal.h"
#include <assert.h>
#include <string.h>


int LIBIMGUTIL_DLL imguPad(struct imgImage **padded, const struct imgImage *src, int top, int bot, int left, int right) {

	struct imgImage *out = NULL;

	if( imgAllocImage( &out ) != IMG_OKAY )
	return -1;

	out->format = src->format;
	out->height = src->height + top + bot;
	out->width = src->width + left + right;

	if( imgAllocPixelBuffers( out ) != IMG_OKAY ) {

		imgFreeAll(out);
		return -1;
	}

	assert( !(src->format & IMG_FMT_COMPONENT_COMPRESSED) && "TODO: impliment compressed paddding" );
	assert( !(src->format & IMG_FMT_COMPONENT_PLANAR) && "TODO: impliment planar padding" );

	{
		int x,y;

		// centre
		imguCopyRect( out, src, left, top, 0, 0, src->width, src->height);

		// top edge
		for(y=0;y<top;y++)
		imguCopyRect( out, src, left, y, 0, 0, src->width, 1);

		// bottom edge
		for(y=src->height+top;y<src->height+top+bot;y++)
		imguCopyRect( out, src, left, y, 0, src->height-1, src->width, 1);

		// left edge
		for(x=0;x<left;x++)
		imguCopyRect( out, src, x, top, 0,0,1, src->height );

		// right edge
		for(x=src->width+left;x<src->width+left+right;x++)
		imguCopyRect( out, src, x, top, src->width-1,0,1, src->height );

		// top left corner
		for(y=0;y<top;y++)
		for(x=0;x<left;x++)
		imguCopyPixel( out, src, x, y, 0, 0 );

		// top right corner
		for(y=0;y<top;y++)
		for(x=src->width+left;x<src->width+left+right;x++)
		imguCopyPixel( out, src, x, y, src->width-1, 0 );

		// bot left corner
		for(y=src->height+top;y<src->height+top+bot;y++)
		for(x=0;x<left;x++)
		imguCopyPixel( out, src, x, y, 0, src->height-1);

		// bot right corner
		for(y=src->height+top;y<src->height+top+bot;y++)
		for(x=src->width+left;x<src->width+left+right;x++)
		imguCopyPixel( out, src, x, y, src->width-1, src->height-1 );

	}

	*padded = out;

	return IMG_OKAY;
}

