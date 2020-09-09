/* Subject 	: A header for handling 24 bit Windows Bitmap(BMP) Image file with 54 byte header
   Date		: 04.03.2011 
   Author	: Rakesh Malik
 */

#ifndef _BMP_24b_54B_H
#define _BMP_24b_54B_H

#include <stdlib.h>

struct rgb_24bit
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char transparent : 1;				//This is not a part of BMP file
};
struct BMP_24b_54B
{
	/* Bitmap file header (14byte) */
	unsigned short 	signature;				// 0x4d42
	unsigned long 	size;
	unsigned short 	reserved1;				// 0
	unsigned short 	reserved2;				// 0
	unsigned long 	offset;
	/* Bitmap info header (40byte) */
	unsigned long 	bitmap_info_header_size;		// 40
	unsigned long 	width;
	unsigned long 	height;
	unsigned short 	planes;					// 1
	unsigned short 	bits_per_pixel;				// 1,4,9,24
	unsigned long 	compression_type;			//0=none,1=RLE8,2=RLE4
	unsigned long	image_data_size;
	unsigned long 	horizontal_resolution_per_meter;
	unsigned long 	vertical_resolution_per_meter;
	unsigned long 	number_of_colors;
	unsigned long 	number_of_important_colors;
	/* Bitmap data (24bit rgb format) */
	struct rgb_24bit **data;
};
void load_BMP_24b_54B(struct BMP_24b_54B *image,const char *fname);		// Load a BMP file
void unload_BMP_24b_54B(struct BMP_24b_54B *image);				// Frees all allocated memory for a loaded BMP file
void print_BMP_24b_54B_info(struct BMP_24b_54B image);				// prints the header of a BMP file


/* Function Definitions */
void load_BMP_24b_54B(struct BMP_24b_54B *image,const char *fname)
{
	int i,j;
	FILE *fp=fopen(fname,"rb");
	if(fname==NULL)
	{
		perror("file read error");
		exit(0);
	}	

	fread(&image->signature,			2,	1,fp);
	fread(&image->size,				4,	1,fp);
	fread(&image->reserved1,			2,	1,fp);
	fread(&image->reserved2,			2,	1,fp);
	fread(&image->offset,				4,	1,fp);
	
	fread(&image->bitmap_info_header_size,		4,	1,fp);
	fread(&image->width,				4,	1,fp);
	fread(&image->height,				4,	1,fp);
	fread(&image->planes,				2,	1,fp);
	fread(&image->bits_per_pixel,			2,	1,fp);
	fread(&image->compression_type,			4,	1,fp);
	fread(&image->image_data_size,			4,	1,fp);
	fread(&image->horizontal_resolution_per_meter,	4,	1,fp);
	fread(&image->vertical_resolution_per_meter,	4,	1,fp);
	fread(&image->number_of_colors,			4,	1,fp);
	fread(&image->number_of_important_colors,	4,	1,fp);
	
	image->data=(struct rgb_24bit**)calloc(sizeof(struct rgb_24bit*),image->height);
	for(i=0;i<image->height;i++)
		image->data[i]=(struct rgb_24bit*)calloc(sizeof(struct rgb_24bit),image->width);
				
	char junk;
	for(i=0;i<image->height;i++)
	{
		for(j=0;j<image->width;j++)
		{			
			fread(&image->data[i][j].blue,sizeof(unsigned char),1,fp);
			fread(&image->data[i][j].green,sizeof(unsigned char),1,fp);
			fread(&image->data[i][j].red,sizeof(unsigned char),1,fp);
			image->data[i][j].transparent=0;
		}
		for(j=0;j<4-(image->width*3)%4;j++)
			fread(&junk,sizeof(char),1,fp);
	}
	
	fclose(fp);
}
void unload_BMP_24b_54B(struct BMP_24b_54B *image)
{
	int i;
	for(i=0;i<image->height;i++)
		free(image->data[i]);
	free(image->data);
}

void print_BMP_24b_54B_info(struct BMP_24b_54B image)
{
	printf("\nBITMAP HEADER FILE :\n");
	printf("Signature : %x\n",image.signature);
	printf("File size : %lu\n",image.size);
	printf("Reserved : %hu\n",image.reserved1);
	printf("Reserved : %hu\n",image.reserved2);
	printf("Offset : %lu\n",image.offset);
	printf("\nBITMAP HEADER INFO :\n");
	printf("Header info size : %lu\n",image.bitmap_info_header_size);
	printf("Resolution in pixel : %lux%lu\n",image.width,image.height);
	printf("Plane : %hu\n",image.planes);
	printf("Bits per pixel : %hu\n",image.bits_per_pixel);
	printf("Compression type : %hu\n",image.compression_type);
	printf("Image data size : %lu\n",image.image_data_size);
	printf("Resolution in meter : %lux%lu\n",image.horizontal_resolution_per_meter,image.vertical_resolution_per_meter);
	printf("Number of colors : %lu\n",image.number_of_colors);
	printf("Number of important colors : %lu\n",image.number_of_important_colors);
}

#endif
