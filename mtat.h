#define HW_14_BONUS_1
#ifndef _MTAT_H_
#define _MTAT_H_
#include "bmp.h"

// Do not modify the declaration of binarize.
BMPImage* binarize(BMPImage* image, int radius, int num_threads, char** a_error);
BMPImage* median(BMPImage* image, int radius, int num_threads, char** error);

// OK to add your own declarations BELOW here
typedef struct{
  int radius;
  int* input;
  int start_idx;
  int count;
  int* output;
  BMPImage* image;
}BNR;

typedef struct{
  int radius;
  unsigned char* input;
  int start_idx;
  int count;
  unsigned char* output;
  BMPImage* image;
}Median;

#endif /* mtat.h */
