#define _GNU_SOURCE

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "logging.h"
#include "parser.h"

int LLVMFuzzerTestOneInput(const uint8_t *data,size_t size)
{
    if (data == NULL || size == 0)
    {
        return 0;
    }

    FILE *file = fmemopen((void *)data,size,"rb");

    if (file == NULL)
    {
        return 0;
    }

    set_logging_enabled(0);
    (void)parse_bitmap(file);

    fclose(file);

    return 0;
}
