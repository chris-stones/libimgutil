

/*
 * compressed_pixel_ops.c
 *
 *  Created on: Feb 23, 2013
 *      Author: cds
 */

#include "internal.h"

#include<txc_dxtn.h>
#include<csquish.h>
#include<assert.h>

#include <pthread.h>

struct imgPixel imgReadCompressed(const struct imgImage *img, int x, int y) {

       unsigned char decompressed[4] = {255,  0, 255, 255} ; // RGBA
  
	struct imgPixel pix = {
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};

	switch( img->format & IMG_FMT_COMPONENT_DXTn) {
	  case IMG_FMT_COMPONENT_DXT1: // no alpha
	    
	    fetch_2d_texel_rgb_dxt1( img->linesize[0], img->data.channel[0], x, y, decompressed );	    
	    break;
	    
	  case IMG_FMT_COMPONENT_DXT3: // sharp alpha
	    
	    fetch_2d_texel_rgba_dxt3( img->linesize[0], img->data.channel[0], x, y, decompressed );
	    break;
	    
	  case IMG_FMT_COMPONENT_DXT5: // gradient alpha
	    
	    fetch_2d_texel_rgba_dxt5( img->linesize[0], img->data.channel[0], x, y, decompressed );
	    break;
	}
	
	pix.red   = ((float)decompressed[0]) / 255.0f;
	pix.green = ((float)decompressed[1]) / 255.0f;
	pix.blue  = ((float)decompressed[2]) / 255.0f;
	pix.alpha = ((float)decompressed[3]) / 255.0f;

	return pix;
}

static int neet_to_reformat(const struct imgImage *dst, const struct imgImage *src) {
 
  if(need_to_pma(dst,src)) 	return 1;
  if(need_to_unpma(dst,src)) 	return 1;
  if(dst->format & IMG_FMT_COMPONENT_ALPHA)
    return ( src->format & IMG_FMT_RGBA32 ) != IMG_FMT_RGBA32;
  return ( src->format & IMG_FMT_RGB24 ) != IMG_FMT_RGB24;
}

static int glFormatToCSquishFormat( GLenum glFmt ) {
  
  switch(glFmt) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      return SQUISH_kDxt1 | SQUISH_kColourIterativeClusterFit;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      return SQUISH_kDxt3 | SQUISH_kColourIterativeClusterFit;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      return SQUISH_kDxt5 | SQUISH_kColourIterativeClusterFit;
  }
  assert(0);
}

static const char * glFormatToString( GLenum glFmt ) {
  
  switch(glFmt) {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      return "SQUISH_kDxt1 | SQUISH_kColourIterativeClusterFit";
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      return "SQUISH_kDxt3 | SQUISH_kColourIterativeClusterFit";
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      return "SQUISH_kDxt5 | SQUISH_kColourIterativeClusterFit";
  }
  assert(0);
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
 
  struct job_struct * job = (struct job_struct *)arg;
  
  squish_CompressImage( job->rgba, job->w, job->h, job->blocks, job->flags, job->metric );
  
  free(job);
  
  return NULL;
}

void squish_CompressImage_mt( int number_of_threads, unsigned char const* rgba, int width, int height, void* blocks, int flags, float* metric ) {
  
  if( number_of_threads > 1 ) {
    
    int thread;
    int h=0;
    int y=0;
    int hblocks = ((((height+3)/4)+number_of_threads)/number_of_threads);
    
    pthread_t  * threads = alloca( sizeof(pthread_t) * number_of_threads );
    
    for(thread=0;thread<number_of_threads;thread++) {
      
      struct job_struct * job = malloc(sizeof( struct job_struct ) );
      
      h = 4 * hblocks;
      if( ( y + h ) > height )
	h = height - y;
      
      job->w = width;
      job->h = h;
      job->flags = flags;
      job->metric = metric;
      job->rgba = rgba + job->w * y * 4;
      
      printf("starting job %d - %d,%d\n", thread, y, h);
        
      if( flags & SQUISH_kDxt1)
	job->blocks = ((char*)blocks) + (((width+3)/4) * ((y+3)/4) *  8 );
      else
	job->blocks = ((char*)blocks) + (((width+3)/4) * ((y+3)/4) * 16 );
      
      y+=h;
      
      pthread_create( threads + thread, NULL, &thread_func, job );
    }
    
    for(; thread>0; thread--)
      pthread_join( threads[thread-1], NULL );
  }
  else
    squish_CompressImage( rgba, width, height, blocks, flags, metric );
  
}

void imgWriteCompressed(struct imgImage *dst, const struct imgImage *csrc) {
  
  // TODO: unlike other pixel operations, this can fail. ENOMEM etc. HANDLE IT
  
  int components;
  int err;
  /////////  UGLY! /////////////////////////////////
  // src is a pointer to const src parameter
  // or a temporary variable copy in a more suitable format.
  struct imgImage vsrc = *csrc;
  struct imgImage *_src = &vsrc;
  struct imgImage *src = _src;
  //////////////////////////////////////////////////
  GLenum gl_fmt;
  
  if((dst->format & IMG_FMT_COMPONENT_DXTn) == IMG_FMT_COMPONENT_DXT1 ) {
    
//  components = 3;
    components = 4;
    
    gl_fmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    
//  if(src->format != IMG_FMT_RGB24) {
    if(src->format != IMG_FMT_RGBA32) {
      
      src = NULL;
      err = imgAllocImage(&src);
      assert(err == IMG_OKAY);
//    src->format = IMG_FMT_RGB24;
      src->format = IMG_FMT_RGBA32;
      src->width = _src->width;
      src->height = _src->height;
      err = imgAllocPixelBuffers(src);
      assert(err == IMG_OKAY);
      err = imguCopyImage( src, _src );
    }
  } else {
   
    components = 4;
    
    switch( dst->format & IMG_FMT_COMPONENT_DXTn) {
      case IMG_FMT_COMPONENT_DXT3:
	gl_fmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	break;
      case IMG_FMT_COMPONENT_DXT5:
	gl_fmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	break;
      default:
	assert(0);
    }
    
    if( neet_to_reformat( dst, src ) ) {
      
      src = NULL;
      err = imgAllocImage(&src);
      assert(err == IMG_OKAY);
      
      src->format = IMG_FMT_RGBA32;
      if(dst->format & IMG_FMT_COMPONENT_PMA)
	src->format |= IMG_FMT_COMPONENT_PMA;
      
      src->width = _src->width;
      src->height = _src->height;
      err = imgAllocPixelBuffers(src);
      assert(err == IMG_OKAY);
      assert((src->format & IMG_FMT_COMPONENT_DXTn) == 0); // imguCopyImage uses this function to handle compressed textures. avoid infinite recursion!
      err = imguCopyImage( src, _src ); 
    }
  }
  
//tx_compress_dxtn( components, src->width, src->height, src->data.channel[0], gl_fmt, dst->data.channel[0], dst->linesize[0] );
//squish_CompressImage( src->data.channel[0], src->width, src->height, dst->data.channel[0], glFormatToCSquishFormat(gl_fmt) ,NULL );
  squish_CompressImage_mt( 8, src->data.channel[0], src->width, src->height, dst->data.channel[0], glFormatToCSquishFormat(gl_fmt) ,NULL );
  
  if(src != _src)
    imgFreeAll(src);
  
}


