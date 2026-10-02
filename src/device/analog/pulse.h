/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_PULSE_H
#define CQ_ANALOG_PULSE_H

#include "analog_datatypes.h"

cq_status init_pulse(pulse *pulse, double duration);
cq_status free_pulse(pulse *pulse);
cq_status play(channel *ch, pulse *pulse);
cq_status capture(channel *ch, pulse *pulse, int *result, int shots);
cq_status delay(channel *ch, double dt);
cq_status barrier(channel **ch, int num_channels);

#endif // CQ_ANALOG_PULSE_H
