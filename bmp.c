#include "bmp.h"
#include <string.h>
#include <stdlib.h>

long int _get_file_size(FILE*);

BMPImage* crop_bmp(BMPImage* image, int x, int y, int w, int h, char** error)
{
	if(w > image->header.width_px)
	{
		char* msg = "w is out of bound";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	if(h > image->header.height_px)
	{
		char* msg = "h is out of bound";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}

	BMPImage* new = malloc(sizeof(BMPImage));
	new->header = image->header;
	new->header.width_px = w;
	new->header.height_px = h;
	int pixel = (new->header.bits_per_pixel / 8);
	int pad = w * pixel;
	while(pad % 4 != 0)
	{
		pad++;
	}
	int pad_ori = image->header.width_px * pixel;
	while(pad_ori % 4 != 0)
	{
		pad_ori++;
	}
	new->header.image_size_bytes = pad * h;
	int size = new->header.image_size_bytes;
	new->data = malloc(sizeof(unsigned char)* (size + 1));
	new->data[size] = '\0';
	int target = (pad_ori * (image->header.height_px - (y + h))) + (x * pixel);
	int temp = 0;
	for(int j = 0; j < h; j++)
	{
		for(int k = 0; k < (w*pixel); k++)
		{
			new->data[temp] = image->data[target + k]; 
			temp++;
		}
		for(int k1 = 0; k1 < (pad - (w*pixel)); k1++)
		{
			new->data[temp] = 0;
			temp++;
		}
		target = pad_ori + target;
	}
	return new;
}


BMPImage* read_bmp(FILE* fp, char**error)
{
	BMPImage* image;
	image = malloc(sizeof(*image) * 1);
	if(fp == NULL)
	{
		free(image);
		char* msg = "fail to read 6x6.bmp";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	if(fread(&(image->header), sizeof(BMPHeader), 1, fp) != 1)
	{
		free(image);
		char* msg = "fread image.header fail";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	if(!check_bmp_header(&(image->header), fp))
	{
		free(image);
		char* msg = "check_bmp_hdr invalid";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	int image_size = image->header.size - sizeof(BMPHeader);
	image->data = malloc(sizeof(*(image->data)) * (image_size));
	if(image->data == NULL)
	{
		free(image->data);
		free(image);
		char* msg = "malloc fail";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	fseek(fp, sizeof(BMPHeader),SEEK_SET);
	if(fread(image->data, image_size, sizeof(char),fp) != 1)
	{
		free(image->data);
		free(image);
		char* msg = "fread image->data fail";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	char lastbyte;
	if(fread(&lastbyte, sizeof(char), 1, fp) != 0)
	{
		free(image->data);
		free(image);
		char* msg = "lastbyte fail";
		*error = malloc(sizeof(**error) * (strlen(msg)+1));
		strcpy(*error, msg);
		return NULL;
	}
	return image;
}

bool check_bmp_header(BMPHeader* bmp_hdr, FILE* fp)
{
	if(bmp_hdr -> type != 0x4d42)
	{
		return false;
	}
	if(bmp_hdr->offset != BMP_HEADER_SIZE)
	{
		return false;
	}
	if(bmp_hdr->dib_header_size != DIB_HEADER_SIZE)
	{
		return false;
	}
	if(bmp_hdr->num_planes != 1)
	{
		return false;
	}
	if(bmp_hdr->compression != 0)
	{
		return false;
	}
	if(bmp_hdr->num_colors != 0 || bmp_hdr->important_colors != 0)
	{
		return false;
	}
	if(bmp_hdr->bits_per_pixel != 16 && bmp_hdr->bits_per_pixel != 24)
	{
		return false;
	}
	long int actualFileSize = _get_file_size(fp);
	if(actualFileSize != bmp_hdr->size)
	{
		return false;
	}
	if(bmp_hdr->image_size_bytes != bmp_hdr->width_px * (bmp_hdr->bits_per_pixel / 8) * bmp_hdr->height_px)
	{
		int pad = bmp_hdr->width_px * (bmp_hdr->bits_per_pixel / 8);
		while(pad % 4 != 0)
		{
			pad++;
		}
		if(bmp_hdr->image_size_bytes != pad * bmp_hdr->height_px)
		{
			return false;
		}
	}
	return true;
}

bool write_bmp(FILE* fp, BMPImage* image, char** error)
{
	if(fp == NULL)
	{
		char* msg = "fopen fail";
		*error = malloc(sizeof(**error) * (strlen(msg) + 1));
		strcpy(*error, msg);
		return false;
	}
	if(fwrite(&(image->header), sizeof(BMPHeader), 1, fp) != 1)
	{
		char* msg = "fwrite image fail";
		*error = malloc(sizeof(**error) * (strlen(msg) + 1));
		strcpy(*error, msg);
		return false;
	}
	if(fwrite(image->data, image->header.image_size_bytes, sizeof(unsigned char), fp) != 1)
	{
		char* msg = "fwrite imagedata fail";
		*error = malloc(sizeof(**error) * (strlen(msg) + 1));
		strcpy(*error, msg);
		return false;
	}
	return true;
}

long int _get_file_size(FILE* fp)
{
	fseek(fp, 0, SEEK_END);
	long int size = ftell(fp);
	return size;
}

void free_bmp(BMPImage* image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
		}
		free(image);
	}
}
