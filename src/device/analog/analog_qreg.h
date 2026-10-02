/**
* @author Mateusz Meller
*
* @copyright Copyright (c) 2025
* UK Research and Innovation,
* Science and Technology Facilities Council,
* Hartree Centre
*/

#ifndef CQ_ANALOG_QREG_H
#define CQ_ANALOG_QREG_H

#include "analog_datatypes.h"

cq_status init_qreg(analog_qreg *qreg, ptrdiff_t qreg_id,
                    ptrdiff_t num_qubits, cq_hamiltonian *hamiltonian);
cq_status init_qubit_pos(qpos *qubit_pos, ptrdiff_t num_qubits);
//cq_status init_qubit_pos(double *qubit_pos, ptrdiff_t num_qubits);
cq_status reset_qreg(analog_qreg *qreg);

cq_status update_qreg_pos(const qpos *new_positions, ptrdiff_t num_qubits, ptrdiff_t qreg_id);
//cq_status update_qreg_pos(const double *new_positions, ptrdiff_t num_qubits, ptrdiff_t qreg_id);

cq_status print_qpos(ptrdiff_t qreg_id);

#endif // CQ_ANALOG_QREG_H
