#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bmp.h"
#include "logging.h"

/*
 * BMP numeric fields are stored in little-endian byte order.
 * This implementation currently assumes a little-endian host.
 */

#define BITMAP_FILE_HEADER_SIZE 14U
#define BITMAP_INFO_HEADER_SIZE 40U
#define MIN_PIXEL_OFFSET (BITMAP_FILE_HEADER_SIZE + BITMAP_INFO_HEADER_SIZE)
#define BI_RGB 0U

/* =========================
 * Private helper functions
 * =========================
 */

static long get_file_size(FILE *file);
static int validate_file_size(long actual_size,uint32_t expected_size);
static int validate_reserved_field(uint16_t value,const char *field_name);
static int validate_pixel_offset(uint32_t pixel_offset,long actual_size);
static int validate_header_size(uint32_t header_size);
static int validate_dimension(int32_t value,const char *field_name);
static int validate_planes(uint16_t planes);
static int validate_bits_per_pixel(uint16_t bits_per_pixel);
static int validate_compression(uint32_t compression);
static int validate_image_size(const BitmapInfoHeader *info_header,BitmapDerivedInfo *derived_info);
static int calculate_image_size(int32_t width,int32_t height,uint16_t bits_per_pixel,BitmapDerivedInfo *derived_info);

/* =========================
 * BITMAPFILEHEADER
 * =========================
 */

/*
 * Reads and validates the 14-byte BMP file header
 * and stores its fields in BitmapFileHeader.
 */
int read_bitmap_file_header(FILE *file,BitmapFileHeader *header,BitmapDerivedInfo *derived_info)
{
    if (file == NULL || header == NULL || derived_info == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: NULL pointer passed to read_bitmap_file_header.\n");
        }

        return 1;
    }

    /*
     * Calculate the actual file size once.
     * It will be used for several header validations.
     */
    long actual_size = get_file_size(file);

    if (actual_size == -1L)
    {
        return 1;
    }

    /* Read the two-byte BMP signature ("BM"). */
    char signature[2];
    size_t bytes_read = fread(signature,1,2,file);

    if (bytes_read != 2)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP signature.\n");
        }

        return 1;
    }

    /* Verify that the file starts with the BMP signature. */
    if (signature[0] != 'B' || signature[1] != 'M')
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: invalid BMP signature.\n");
        }

        return 1;
    }

    /*
     * Store the two signature bytes as one uint16_t value.
     * The first byte is the low byte and the second is the high byte.
     */
    uint16_t low_byte = (unsigned char)signature[0];
    uint16_t high_byte = (uint16_t)(unsigned char)signature[1] << 8;
    header->type = low_byte | high_byte;

    /*
     * Read bfSize directly into the structure.
     * Direct numeric reads assume a little-endian host.
     */
    size_t items_read = fread(&header->file_size,sizeof(header->file_size),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP file size.\n");
        }

        return 1;
    }

    /* Verify that bfSize matches the actual file size. */
    int result = validate_file_size(actual_size,header->file_size);

    if (result != 0)
    {
        return 1;
    }

    derived_info->actual_file_size = (uint32_t)actual_size;

    /* Read bfReserved1. */
    items_read = fread(&header->reserved1,sizeof(header->reserved1),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP reserved1.\n");
        }

        return 1;
    }

    /* Reserved fields must be zero. */
    result = validate_reserved_field(header->reserved1,"bfReserved1");

    if (result != 0)
    {
        return 1;
    }

    /* Read bfReserved2. */
    items_read = fread(&header->reserved2,sizeof(header->reserved2),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP reserved2.\n");
        }

        return 1;
    }

    /* Reserved fields must be zero. */
    result = validate_reserved_field(header->reserved2,"bfReserved2");

    if (result != 0)
    {
        return 1;
    }

    /* Read bfOffBits: offset at which the pixel array begins. */
    items_read = fread(&header->pixel_offset,sizeof(header->pixel_offset),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP pixel offset.\n");
        }

        return 1;
    }

    /* Verify that the pixel array starts at a valid position. */
    result = validate_pixel_offset(header->pixel_offset,actual_size);

    if (result != 0)
    {
        return 1;
    }

    return 0;
}

/* =========================
 * BITMAPINFOHEADER
 * =========================
 */

/*
 * Reads and validates the 40-byte BITMAPINFOHEADER.
 * Derived values calculated from the header are stored in derived_info.
 */
int read_bitmap_info_header(FILE *file,BitmapInfoHeader *info_header,BitmapDerivedInfo *derived_info)
{
    if (file == NULL || info_header == NULL || derived_info == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: NULL pointer passed to read_bitmap_info_header.\n");
        }

        return 1;
    }

    /* Read the DIB header size. */
    size_t items_read = fread(&info_header->header_size,sizeof(info_header->header_size),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP info header size.\n");
        }

        return 1;
    }

    /* Verify that the DIB header size is supported. */
    int result = validate_header_size(info_header->header_size);

    if (result != 0)
    {
        return 1;
    }

    /* Read and validate the bitmap width. */
    items_read = fread(&info_header->width,sizeof(info_header->width),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP width.\n");
        }

        return 1;
    }

    result = validate_dimension(info_header->width,"width");

    if (result != 0)
    {
        return 1;
    }

    /* Read and validate the bitmap height. */
    items_read = fread(&info_header->height,sizeof(info_header->height),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP height.\n");
        }

        return 1;
    }

    result = validate_dimension(info_header->height,"height");

    if (result != 0)
    {
        return 1;
    }

    /* Read the number of color planes. */
    items_read = fread(&info_header->planes,sizeof(info_header->planes),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP planes.\n");
        }

        return 1;
    }

    result = validate_planes(info_header->planes);

    if (result != 0)
    {
        return 1;
    }

    /* Read the number of bits per pixel. */
    items_read = fread(&info_header->bits_per_pixel,sizeof(info_header->bits_per_pixel),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP bits per pixel.\n");
        }

        return 1;
    }

    result = validate_bits_per_pixel(info_header->bits_per_pixel);

    if (result != 0)
    {
        return 1;
    }

    /* Read and validate the BMP compression method. */
    items_read = fread(&info_header->compression,sizeof(info_header->compression),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP compression.\n");
        }

        return 1;
    }

    /* Verify that the BMP image uses the BI_RGB uncompressed format. */
    result = validate_compression(info_header->compression);

    if (result != 0)
    {
        return 1;
    }

    /* Read the BMP image size. */
    items_read = fread(&info_header->image_size,sizeof(info_header->image_size),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP image size.\n");
        }

        return 1;
    }

    /* Validate biSizeImage and calculate the actual pixel data size. */
    result = validate_image_size(info_header,derived_info);

    if (result != 0)
    {
        return 1;
    }

    /* Read the horizontal resolution in pixels per meter. */
    items_read = fread(&info_header->x_pixels_per_meter,sizeof(info_header->x_pixels_per_meter),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP x_pixels_per_meter.\n");
        }

        return 1;
    }

    /* Read the vertical resolution in pixels per meter. */
    items_read = fread(&info_header->y_pixels_per_meter,sizeof(info_header->y_pixels_per_meter),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP y_pixels_per_meter.\n");
        }

        return 1;
    }

    /* Read the number of color-table entries used by the bitmap. */
    items_read = fread(&info_header->colors_used,sizeof(info_header->colors_used),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP colors used.\n");
        }

        return 1;
    }

    /* Read the number of colors considered important for display. */
    items_read = fread(&info_header->important_colors,sizeof(info_header->important_colors),1,file);

    if (items_read != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to read BMP important colors.\n");
        }

        return 1;
    }

    return 0;
}

/* =========================
 * Helper function implementations
 * =========================
 */

/*
 * Returns the actual size of the file without changing
 * the caller's current file position.
 *
 * Returns:
 * File size in bytes on success.
 * -1L on failure.
 */
static long get_file_size(FILE *file)
{
    if (file == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: NULL file pointer passed to get_file_size.\n");
        }

        return -1L;
    }

    /* Save the current position so it can be restored later. */
    long original_position = ftell(file);

    if (original_position == -1L)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to get current file position.\n");
        }

        return -1L;
    }

    /* Move to the end of the file. */
    int result = fseek(file,0,SEEK_END);

    if (result != 0)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to seek to end of file.\n");
        }

        return -1L;
    }

    /* At the end of the file, the current position represents the file size in bytes. */
    long actual_size = ftell(file);

    if (actual_size == -1L)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to get file size.\n");
        }

        return -1L;
    }

    /* Restore the caller's original file position. */
    result = fseek(file,original_position,SEEK_SET);

    if (result != 0)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: failed to restore file position.\n");
        }

        return -1L;
    }

    return actual_size;
}

/* Verifies that bfSize matches the actual file size. */
static int validate_file_size(long actual_size,uint32_t expected_size)
{
    if (actual_size < 0 || (uint64_t)actual_size > UINT32_MAX)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: actual BMP file size cannot be represented by bfSize.\n");
        }

        return 1;
    }

    if ((uint32_t)actual_size != expected_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: BMP file size does not match actual file size.\n");
        }

        return 1;
    }

    return 0;
}

/* Validates that a reserved BMP header field is zero. */
static int validate_reserved_field(uint16_t value,const char *field_name)
{
    if (field_name == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: reserved field name is NULL.\n");
        }

        return 1;
    }

    if (value != 0)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: %s must be zero.\n",field_name);
        }

        return 1;
    }

    return 0;
}

/* Validates that the pixel array offset is within the supported BMP file range. */
static int validate_pixel_offset(uint32_t pixel_offset,long actual_size)
{
    if (actual_size < 0 || (uint64_t)actual_size > UINT32_MAX)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: invalid BMP file size while validating pixel offset.\n");
        }

        return 1;
    }

    /*
     * For the BMP variant currently supported:
     * BITMAPFILEHEADER = 14 bytes.
     * BITMAPINFOHEADER = 40 bytes.
     * Therefore the pixel array cannot begin before byte 54.
     */
    if (pixel_offset < MIN_PIXEL_OFFSET || pixel_offset > (uint32_t)actual_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: invalid BMP pixel array offset.\n");
        }

        return 1;
    }

    return 0;
}

/* Verify that the DIB header size matches BITMAPINFOHEADER. */
static int validate_header_size(uint32_t header_size)
{
    if (header_size != BITMAP_INFO_HEADER_SIZE)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: unsupported DIB header size.\n");
        }

        return 1;
    }

    return 0;
}

/*
 * Validates a BMP image dimension.
 * Width must be positive.
 * Height may be positive or negative, but cannot be zero.
 */
static int validate_dimension(int32_t value,const char *field_name)
{
    if (field_name == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: dimension field name is NULL.\n");
        }

        return 1;
    }

    if (strcmp(field_name,"height") == 0)
    {
        if (value == 0)
        {
            if (is_logging_enabled())
            {
                fprintf(stderr,"Error: BMP height cannot be zero.\n");
            }

            return 1;
        }

        return 0;
    }

    if (strcmp(field_name,"width") == 0)
    {
        if (value <= 0)
        {
            if (is_logging_enabled())
            {
                fprintf(stderr,"Error: BMP width cannot be zero or negative.\n");
            }

            return 1;
        }

        return 0;
    }

    if (is_logging_enabled())
    {
        fprintf(stderr,"Error: unknown dimension field.\n");
    }

    return 1;
}

/* Verify that the BMP image contains exactly one color plane. */
static int validate_planes(uint16_t planes)
{
    if (planes != 1)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: BMP planes field must be 1.\n");
        }

        return 1;
    }

    return 0;
}

/* Verify that the bitmap uses the currently supported 24-bit pixel format. */
static int validate_bits_per_pixel(uint16_t bits_per_pixel)
{
    if (bits_per_pixel != 24)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: only 24-bit BMP images are currently supported.\n");
        }

        return 1;
    }

    return 0;
}

/* Verify that the BMP image uses the BI_RGB uncompressed format. */
static int validate_compression(uint32_t compression)
{
    if (compression != BI_RGB)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: only uncompressed BI_RGB BMP images are currently supported.\n");
        }

        return 1;
    }

    return 0;
}

/*
 * Validate the BMP image size against the size calculated from
 * the image dimensions and pixel format.
 *
 * For uncompressed BI_RGB bitmaps, biSizeImage may be zero.
 * The calculated size is stored in derived_info for later use.
 */
static int validate_image_size(const BitmapInfoHeader *info_header,BitmapDerivedInfo *derived_info)
{
    if (info_header == NULL || derived_info == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: NULL pointer passed to validate_image_size.\n");
        }

        return 1;
    }

    /* Calculate the expected pixel array size and derived layout information. */
    int result = calculate_image_size(info_header->width,info_header->height,info_header->bits_per_pixel,derived_info);

    if (result != 0)
    {
        return 1;
    }

    /* For uncompressed BI_RGB bitmaps, the BMP specification allows biSizeImage to be zero. */
    if (info_header->image_size == 0)
    {
        return 0;
    }

    /* Verify that the image size stored in the header matches our calculated size. */
    if (info_header->image_size != derived_info->calculated_image_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: BMP image size does not match calculated image size.\n");
        }

        return 1;
    }

    return 0;
}

/*
 * Calculate derived values for the BMP pixel data, including row padding.
 * Arithmetic operations are checked to prevent integer overflow.
 * The calculated values are stored in derived_info.
 */
static int calculate_image_size(int32_t width,int32_t height,uint16_t bits_per_pixel,BitmapDerivedInfo *derived_info)
{
    if (derived_info == NULL)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: derived_info output pointer is NULL.\n");
        }

        return 1;
    }

    /*
     * This helper currently relies on the supported BMP subset:
     * width must be positive, height must be non-zero and bits_per_pixel must be 24.
     */
    if (width <= 0 || height == 0 || bits_per_pixel == 0 || bits_per_pixel % 8 != 0)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: invalid values passed to BMP image size calculation.\n");
        }

        return 1;
    }

    uint32_t bytes_per_pixel = bits_per_pixel / 8;

    /* Check that width * bytes_per_pixel does not overflow. */
    if ((uint32_t)width > UINT32_MAX / bytes_per_pixel)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: BMP row size calculation would overflow.\n");
        }

        return 1;
    }

    uint32_t row_size = (uint32_t)width * bytes_per_pixel;

    /* BMP rows are padded to a multiple of 4 bytes. */
    uint32_t remainder = row_size % 4;
    uint32_t padding = 0;

    if (remainder != 0)
    {
        padding = 4 - remainder;
    }

    /* Check that adding the row padding does not overflow. */
    if (row_size > UINT32_MAX - padding)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: BMP padded row size calculation would overflow.\n");
        }

        return 1;
    }

    uint32_t padded_row_size = row_size + padding;

    /*
     * Convert the signed BMP height to the actual number of rows.
     * A negative height represents a top-down bitmap.
     */
    uint32_t number_of_rows;

    if (height > 0)
    {
        number_of_rows = (uint32_t)height;
    }
    else
    {
        int64_t height_64 = (int64_t)height;
        height_64 = -height_64;
        number_of_rows = (uint32_t)height_64;
    }

    /* Check that padded_row_size * number_of_rows does not overflow. */
    if (number_of_rows > UINT32_MAX / padded_row_size)
    {
        if (is_logging_enabled())
        {
            fprintf(stderr,"Error: BMP image size calculation would overflow.\n");
        }

        return 1;
    }

    derived_info->bytes_per_pixel = bytes_per_pixel;
    derived_info->padding = padding;
    derived_info->padded_row_size = padded_row_size;
    derived_info->number_of_rows = number_of_rows;
    derived_info->calculated_image_size = number_of_rows * padded_row_size;

    return 0;
}
