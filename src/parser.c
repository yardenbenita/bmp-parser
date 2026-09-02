#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "pixel_data.h"
#include "parser.h"

int parse_bitmap(FILE *file)
{
    if (file == NULL)
    {
        fprintf(stderr, "Error: NULL file pointer passed to parse_bitmap.\n");
        return 1;
    }

    BitmapFileHeader header;
    BitmapInfoHeader info_header;
    BitmapDerivedInfo derived_info;
    BitmapPixelData pixel_data = {NULL, 0};

    if (read_bitmap_file_header(file, &header, &derived_info) != 0)
    {
        return 1;
    }

    if (read_bitmap_info_header(file, &info_header, &derived_info) != 0)
    {
        return 1;
    }

    if (read_pixel_data(file, header.pixel_offset, &derived_info, &info_header, &pixel_data) != 0)
    {
        return 1;
    }

    if (process_pixel_data(&pixel_data, &info_header, &derived_info) != 0)
    {
        free(pixel_data.data);
        pixel_data.data = NULL;
        pixel_data.size = 0;
        return 1;
    }

    free(pixel_data.data);
    pixel_data.data = NULL;
    pixel_data.size = 0;

    return 0;
}