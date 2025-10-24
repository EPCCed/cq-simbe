/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_CHANNEL_H
#define CQ_ANALOG_CHANNEL_H

#include "analog_datatypes.h"

cq_status setup_channel_params(analog_qreg *qreg);
cq_status copy_channel(channel *dest, const channel *src);
cq_status retarget_channel(channel *ch, ptrdiff_t new_target);
void print_avail_channels(void);
void print_channel(channel *ch);

#endif //CQ_ANALOG_CHANNEL_H
