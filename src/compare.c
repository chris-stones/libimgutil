


#include "internal.h"
#include <string.h>

int imguBinaryCompare(struct imgImage *a,struct imgImage *b) {
  
  int plane;
  
  if(!a || !b)
    return -1;
  if(a == b)
    return 0;
  if(a->width != b->width)
    return 1;
  if(a->height != b->height)
    return 1;
  
  for(plane=0;plane<4;plane++) {
    if(a->linearsize[plane] != b->linearsize[plane])
      return 1;
    if(memcmp(a->data.channel[plane],b->data.channel[plane],a->linearsize[plane]) != 0)
      return 1;
  }
  
  return 0;
}


