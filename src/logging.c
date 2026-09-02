#include "logging.h"

static int logging_enabled = 1;

void set_logging_enabled(int enabled)
{
    logging_enabled = enabled;
}

int is_logging_enabled(void)
{
    return logging_enabled;
}
