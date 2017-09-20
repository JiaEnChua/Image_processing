#include "mtat.h"
#include <stdlib.h>

int main(int argc, const char *argv[])
{
	FILE *fp;
//	fp = fopen("img1_6x6_color.bmp","rb");
//	fp = fopen("img2_384x510_gray.bmp","rb");
	fp = fopen("car.bmp","rb");
	char* error = NULL;
	BMPImage* image;
//	BMPImage* image1;
	BMPImage* image2;
	image = read_bmp(fp, &error);
	if(image == NULL)
	{
		fprintf(stderr, error);
		free(error);
		return EXIT_FAILURE;
	}
	if(fp != NULL)
	{
		fclose(fp);
	}
/*	image1 = binarize(image, 1, 2, &error);
	if(image1 == NULL)
	{
		fprintf(stderr, error);
		free(error);
		return EXIT_FAILURE;
	}*/
	image2 = median(image, 1, 2, &error);
	if(image2 == NULL)
	{
		fprintf(stderr, error);
		free(error);
		return EXIT_FAILURE;
	}
	FILE *fp2;
	fp2 = fopen("6x6gray.bmp","wb");
	if(!write_bmp(fp2, image2, &error))
	{
		fprintf(stderr, error);
		free(error);
		return EXIT_FAILURE;
	}
	if(fp2 != NULL)
	{
		fclose(fp2);
	}
	free_bmp(image);
//	free_bmp(image1);
	free_bmp(image2);
	return EXIT_SUCCESS;
}
