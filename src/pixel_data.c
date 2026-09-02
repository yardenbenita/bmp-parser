#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "pixel_data.h"
#include "logging.h"

/*
 * Reads the BMP pixel array into dynamically allocated memory.
 * The function verifies that the expected pixel data fits within the file,
 * seeks to the pixel array offset, allocates the required buffer,
 * and verifies that the complete pixel array was read successfully.
 */
int read_pixel_data(FILE *file,uint32_t pixel_offset,const BitmapDerivedInfo *derived_info,const BitmapInfoHeader *info_header,BitmapPixelData *pixel_data)
{
    /* Validate pointer arguments before dereferencing or using them. */
    if (file == NULL || info_header == NULL || pixel_data == NULL || derived_info == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: invalid argument passed to read_pixel_data.\n");
        }

        return 1;
    }

    /* Verify that the pixel array offset does not point beyond the end of the file. */
    if (pixel_offset > derived_info->actual_file_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: pixel data offset is beyond the end of the file.\n");
        }

        return 1;
    }

    /* Calculate the number of bytes available from the pixel array offset to the end of the file. */
    uint32_t available_bytes = derived_info->actual_file_size - pixel_offset;

    /* Verify that the file contains enough bytes for the expected pixel array. */
    if (available_bytes < derived_info->calculated_image_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: file does not contain enough data for the expected pixel array.\n");
        }

        return 1;
    }

    /* Move the file position to the beginning of the pixel array. */
    if (fseek(file,pixel_offset,SEEK_SET) != 0)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to seek to pixel data offset.\n");
        }

        return 1;
    }

    /* Allocate memory for the complete pixel array. */
    pixel_data->data = malloc(derived_info->calculated_image_size);

    if (pixel_data->data == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to allocate memory for pixel data.\n");
        }

        return 1;
    }

    /* Read the complete pixel array into the allocated buffer. */
    size_t bytes_read = fread(pixel_data->data,1,derived_info->calculated_image_size,file);

    if (bytes_read != derived_info->calculated_image_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read the complete pixel data.\n");
        }

        free(pixel_data->data);
        pixel_data->data = NULL;
        pixel_data->size = 0;
        return 1;
    }

    /* Store the size of the successfully loaded pixel buffer. */
    pixel_data->size = derived_info->calculated_image_size;

    return 0;
}

/*
 * Processes the loaded BMP pixel array without modifying it.
 * The function walks through the image row by row and pixel by pixel,
 * interprets each 24-bit pixel in BGR order, and prints a few sample pixels
 * from the first row to verify that the pixel data was parsed correctly.
 */
int process_pixel_data(const BitmapPixelData *pixel_data,const BitmapInfoHeader *info_header,const BitmapDerivedInfo *derived_info)
{
    /* Validate pointer arguments before accessing their fields. */
    if (pixel_data == NULL || info_header == NULL || derived_info == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: invalid argument passed to process_pixel_data.\n");
        }

        return 1;
    }

    /* Verify that the pixel buffer was successfully loaded. */
    if (pixel_data->data == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: pixel data buffer is NULL.\n");
        }

        return 1;
    }

    uint32_t pixel_offset_in_buffer;
    uint8_t blue;
    uint8_t green;
    uint8_t red;

    /* Walk through every row in the loaded pixel array. */
    for (uint32_t row = 0; row < derived_info->number_of_rows; row++)
    {
        /* Walk through every pixel in the current row. */
        for (uint32_t column = 0; column < (uint32_t)info_header->width; column++)
        {
            /* Calculate the byte offset of the current pixel within the pixel buffer. */
            pixel_offset_in_buffer = row * derived_info->padded_row_size + column * derived_info->bytes_per_pixel;

            /* A 24-bit BI_RGB pixel is stored in BGR byte order. */
            blue = pixel_data->data[pixel_offset_in_buffer];
            green = pixel_data->data[pixel_offset_in_buffer + 1];
            red = pixel_data->data[pixel_offset_in_buffer + 2];

            /* Print a few sample pixels from the first stored row for verification. */
            if (is_logging_enabled() && row == 0 && column < 5)
            {
                printf("Pixel (%u,%u): B=%u G=%u R=%u\n",column,row,blue,green,red);
            }
        }
    }

    return 0;
}
