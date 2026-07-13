#ifndef CQ_COMMS_UTILS_H
#define CQ_COMMS_UTILS_H

#include <stdbool.h>

bool is_device();

#define CQ_PROG_BEGIN() if (!is_device()) {
#define CQ_PROG_END() }

#endif
