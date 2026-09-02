#ifndef PIXEL_DATA_H
#define PIXEL_DATA_H

#include <stdio.h>
#include <stdint.h>
#include "bmp.h"

/*
 * Stores the bitmap pixel data read from the file.
 */
typedef struct
{
 uint8_t *data;   /* Pointer to the pixel data buffer. */
 uint32_t size;   /* Size of the pixel data buffer in bytes. */
} BitmapPixelData;

/*
 * Reads and validates the bitmap pixel data.
 */
int read_pixel_data(FILE *file,uint32_t pixel_offset,const BitmapDerivedInfo *derived_info,const BitmapInfoHeader *info_header,BitmapPixelData *pixel_data);

int process_pixel_data(const BitmapPixelData *pixel_data,const BitmapInfoHeader *info_header,const BitmapDerivedInfo *derived_info);
#endif