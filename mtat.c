#include "mtat.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void _addH(int*, int, double*, int*, int, int);
static int sumPixel(BMPImage*, int*, int, int);
static int sumMedian(BMPImage* image, unsigned char* input, int start_idx, int radius);
int compare_fn(const void* a, const void* b);
int _num_Pixel(BMPImage* image, unsigned char* input, int start_idx, int radius);
void _getpixel(unsigned char* input, int index,int *num_pixel, int radius, int width);
void _fillmedian(unsigned char* input, int index, unsigned char* tobesortedArray, int *num_pixel, int radius, int width);


void _addH(int* input, int index, double* sum, int* num_pixel, int radius, int width)
{
	for(int i = 1;i <= radius;i++)
	{
		if((index % width) + i < width)
		{
			*sum += input[index + i];
			*num_pixel = *num_pixel + 1;
		}
		if((index % width) - i < width && ((index % width) - i) >= 0)
		{
			*sum += input[index - i];
			*num_pixel = *num_pixel + 1;
		}
	}
}

static int sumPixel(BMPImage* image, int* input, int start_idx, int radius)
{
	double sum = input[start_idx];
	int width = image->header.width_px;
	int num_pixel = 1;
	_addH(input, start_idx, &sum, &num_pixel, radius, width);
	for(int j = 1;j <= radius;j++)
	{
		int coordinate = start_idx + (j * width);
		if(coordinate < (image->header.width_px * image->header.width_px))
		{
			sum += input[coordinate];
			num_pixel++;
			_addH(input, coordinate, &sum, &num_pixel, radius, width);
		}
		coordinate = start_idx - (j * width);
		if(coordinate >= 0)
		{
			sum += input[coordinate];
			num_pixel++;
			_addH(input, coordinate, &sum, &num_pixel, radius, width);
		}
	}
	int temp = num_pixel * input[start_idx];
	if(temp <= sum)
	{
		sum = 0;
	}
	else
	{
		sum = 255;
	}
	return sum;
}

void* _binarize(void* arg)
{
	BNR* thread = (BNR*) arg;
	for(int i = 0; i < thread->count; i++)
	{
		thread->output[thread->start_idx] = sumPixel(thread->image, thread->input, thread->start_idx, thread->radius);
		thread->start_idx++;
	}
	return NULL;
}

BMPImage* binarize(BMPImage* image, int radius, int num_threads, char** error)
{
	int w = image->header.width_px;
	int pixel_with_pad = w * 3;
	while(pixel_with_pad % 4 != 0)
	{
		pixel_with_pad++;
	}
	int padding = (pixel_with_pad - w*3);
	int numpixel = image->header.width_px * image->header.height_px;
	int count = numpixel / num_threads;
	int remainder = numpixel % num_threads;
	int i1 = 0;
	int* pixel = malloc(sizeof(int) * numpixel);
	int* output = malloc(sizeof(int) * numpixel);
	for(int i = 0; i < numpixel; i++)
	{
		pixel[i] = image->data[i1] + image->data[i1+1] +image->data[i1+2];
		i1 += 3;
		if(i != 0 && (i+1) % w == 0)
		{
			for(int j = 0; j < padding; j++)
			{
				i1++;
			}
		}
	}
	BMPImage* img = crop_bmp(image, 0, 0, image->header.width_px, image->header.height_px, error);
	BNR *thread;
	thread = malloc(sizeof(*thread) * num_threads);	
	pthread_t *ptr;
	ptr = malloc(sizeof(*ptr) * num_threads);
	int rtv;
	for(int i = 0; i < num_threads; i++)
	{
		if(i == 0)
		{
			thread[i].start_idx = 0;
		}
		if(remainder != 0)
		{
			count++;	
			remainder--;
			thread[i].count = count;
			if(i != 0)
			{
				thread[i].start_idx = thread[i-1].start_idx + thread[i-1].count;
			}
			count--;
		}
		else
		{
			thread[i].count = count;
			if(i != 0)
			{
				thread[i].start_idx = thread[i-1].start_idx + thread[i-1].count;
			}
		}
		thread[i].radius = radius;
		thread[i].input = pixel;
		thread[i].output = output;
		thread[i].image = image;
		if(i > 0)
		{
			for(int j = 0; j < numpixel; j++)
			{
				thread[i].output[j] = thread[i-1].output[j];
			}
		}

		rtv = pthread_create(&ptr[i], NULL, _binarize, (void*) &thread[i]);
		if(rtv != 0)
		{
			char* msg = "Error! pthread_create fail";
			*error = malloc(sizeof(char) * strlen(msg + 1));
			strcpy(*error, msg);
			free_bmp(img);
			free(pixel);
			free(output);
			free(ptr);
			free(thread);
			return NULL;
		}
	}
	for(int i = 0; i < num_threads; i++)
	{
		rtv = pthread_join(ptr[i], NULL);
		if(rtv != 0)
		{
			char* msg = "Error! pthread_create fail";
			*error = malloc(sizeof(char) * strlen(msg + 1));
			strcpy(*error, msg);
			free_bmp(img);
			free(pixel);
			free(output);
			free(ptr);
			free(thread);
			return NULL;
		}
	}
	int i2 = 0;
	for(int i = 0; i < numpixel; i++)
	{
		img->data[i2] = thread[num_threads - 1].output[i];
		img->data[i2 + 1] = thread[num_threads - 1].output[i];
		img->data[i2 + 2] = thread[num_threads - 1].output[i];
		i2 += 3;
		if(i != 0 && (i+1) % w == 0)
		{
			for(int j = 0; j < padding; j++)
			{
				img->data[i2] = 0;
				i2++;
			}
		}
	}
	
	free(pixel);
	free(output);
	free(ptr);
	free(thread);
	return img;
}

void* _median(void* arg)
{
	Median* thread = (Median*) arg;
	for(int i = 0; i < thread->count; i++)
	{
		thread->output[thread->start_idx] = sumMedian(thread->image, thread->input, thread->start_idx, thread->radius);
		thread->start_idx++;
	}
	return NULL;
}

BMPImage* median(BMPImage* image, int radius, int num_threads, char** error)
{
	int size = image->header.image_size_bytes;
	int count = size / num_threads;
	int remainder = size % num_threads;
	BMPImage* img = crop_bmp(image, 0, 0, image->header.width_px, image->header.height_px, error);
	Median *thread;
	thread = malloc(sizeof(*thread) * num_threads);	
	pthread_t *ptr;
	ptr = malloc(sizeof(*ptr) * num_threads);
	int rtv;
	for(int i = 0; i < num_threads; i++)
	{
		if(i == 0)
		{
			thread[i].start_idx = 0;
		}
		if(remainder != 0)
		{
			count++;	
			remainder--;
			thread[i].count = count;
			if(i != 0)
			{
				thread[i].start_idx = thread[i-1].start_idx + thread[i-1].count;
			}
			count--;
		}
		else
		{
			thread[i].count = count;
			if(i != 0)
			{
				thread[i].start_idx = thread[i-1].start_idx + thread[i-1].count;
			}
		}
		thread[i].radius = radius;
		thread[i].input = image->data;
		thread[i].output = img->data;
		thread[i].image = image;
		if(i > 0)
		{
			for(int j = 0; j < size; j++)
			{
				thread[i].output[j] = thread[i-1].output[j];
			}
		}

		rtv = pthread_create(&ptr[i], NULL, _median, (void*) &thread[i]);
		if(rtv != 0)
		{
			char* msg = "Error! pthread_create fail";
			*error = malloc(sizeof(char) * strlen(msg + 1));
			strcpy(*error, msg);
			free_bmp(img);
			free(ptr);
			free(thread);
			return NULL;
		}
	}
	for(int i = 0; i < num_threads; i++)
	{
		rtv = pthread_join(ptr[i], NULL);
		if(rtv != 0)
		{
			char* msg = "Error! pthread_create fail";
			*error = malloc(sizeof(char) * strlen(msg + 1));
			strcpy(*error, msg);
			free_bmp(img);
			free(ptr);
			free(thread);
			return NULL;
		}
	}
	
	free(ptr);
	free(thread);
	return img;
}

static int sumMedian(BMPImage* image, unsigned char* input, int start_idx, int radius)
{
//	printf("%d)inputvalue: %d\n", start_idx, input[start_idx]);
	int size = _num_Pixel(image, input, start_idx, radius);
 	unsigned char* tobesortedArray = malloc(sizeof(*tobesortedArray) * size);	
	int width = image->header.width_px;
	int num_pixel = 0;
	tobesortedArray[num_pixel] = input[start_idx];
	num_pixel++;
	_fillmedian(input, start_idx, tobesortedArray, &num_pixel, radius, width);
	for(int j = 1;j <= radius;j++)
	{
		int coordinate = start_idx + (j * width);
		if(coordinate < (image->header.width_px * image->header.width_px))
		{
			tobesortedArray[num_pixel] = input[coordinate];
			num_pixel++;
			_fillmedian(input, coordinate, tobesortedArray, &num_pixel, radius, width);
		}
		coordinate = start_idx - (j * width);
		if(coordinate >= 0)
		{
			tobesortedArray[num_pixel] = input[coordinate];
			num_pixel++;
			_fillmedian(input, coordinate, tobesortedArray, &num_pixel, radius, width);
		}
	}
	qsort(tobesortedArray, size, sizeof(*tobesortedArray), compare_fn);
//	printf("return value: %d\n",tobesortedArray[size/2]);
	return tobesortedArray[(size/2) + 1];
}

void _fillmedian(unsigned char* input, int index, unsigned char* tobesortedArray, int *num_pixel, int radius, int width)
{
	for(int i = 1;i <= radius;i++)
	{
		if((index % width) + i < width)
		{
			tobesortedArray[*num_pixel] = input[index + i];
			*num_pixel = *num_pixel + 1;
		}
		if((index % width) - i < width && ((index % width) - i) >= 0)
		{
			tobesortedArray[*num_pixel] = input[index - i];
			*num_pixel = *num_pixel + 1;
		}
	}
}

int compare_fn(const void* a, const void* b)
{
	int x = *(int*) a;
	int y = *(int*) b;
	return(x-y);
}

int _num_Pixel(BMPImage* image, unsigned char* input, int start_idx, int radius)
{
	int width = image->header.width_px;
	int num_pixel = 1;
	_getpixel(input, start_idx, &num_pixel, radius, width);
	for(int j = 1;j <= radius;j++)
	{
		int coordinate = start_idx + (j * width);
		if(coordinate < (image->header.width_px * image->header.width_px))
		{
			num_pixel++;
			_getpixel(input, coordinate, &num_pixel, radius, width);
		}
		coordinate = start_idx - (j * width);
		if(coordinate >= 0)
		{
			num_pixel++;
			_getpixel(input, coordinate, &num_pixel, radius, width);
		}
	}
	return num_pixel;
}

void _getpixel(unsigned char* input, int index,int *num_pixel, int radius, int width)
{
	for(int i = 1;i <= radius;i++)
	{
		if((index % width) + i < width)
		{
			*num_pixel = *num_pixel + 1;
		}
		if((index % width) - i < width && ((index % width) - i) >= 0)
		{
			*num_pixel = *num_pixel + 1;
		}
	}
}
