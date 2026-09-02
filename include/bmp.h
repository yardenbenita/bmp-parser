#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <stdint.h>

/*
 * Represents the fields of the 14-byte BITMAPFILEHEADER.
 */
typedef struct
{
    uint16_t type;          /* bfType */
    uint32_t file_size;     /* bfSize */
    uint16_t reserved1;     /* bfReserved1 */
    uint16_t reserved2;     /* bfReserved2 */
    uint32_t pixel_offset;  /* bfOffBits */
} BitmapFileHeader;

/*
 * Stores values calculated by the parser from the BMP header fields,
 * rather than values read directly from the file.
 */
typedef struct
{
    uint32_t actual_file_size;
    uint32_t calculated_image_size;
    uint32_t bytes_per_pixel;
    uint32_t padding;
    uint32_t padded_row_size;
    uint32_t number_of_rows;
} BitmapDerivedInfo;
/*
 * Represents the fields of the 40-byte BITMAPINFOHEADER.
 */
typedef struct
{
    uint32_t header_size;          /* biSize */
    int32_t width;                 /* biWidth */
    int32_t height;                /* biHeight */
    uint16_t planes;               /* biPlanes */
    uint16_t bits_per_pixel;       /* biBitCount */
    uint32_t compression;          /* biCompression */
    uint32_t image_size;           /* biSizeImage */
    int32_t x_pixels_per_meter;    /* biXPelsPerMeter */
    int32_t y_pixels_per_meter;    /* biYPelsPerMeter */
    uint32_t colors_used;          /* biClrUsed */
    uint32_t important_colors;     /* biClrImportant */
} BitmapInfoHeader;


/*
 * Reads and validates the BITMAPFILEHEADER.
 */
int read_bitmap_file_header(FILE *file,BitmapFileHeader *header,BitmapDerivedInfo *derived_info);
/*
 * Reads and validates the BITMAPINFOHEADER and stores derived values.
 */
int read_bitmap_info_header(FILE *file,BitmapInfoHeader *info_header,BitmapDerivedInfo *derived_info);

#endif