#ifndef VQE_UTILS_H
#define VQE_UTILS_H

#include "cq.h"

#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

#define NLAYERS 1
#define MAX_NQUBITS 2
#define NPARAMS NLAYERS * MAX_NQUBITS
#define PARAM_MIN 0.0
#define PARAM_MAX 2.0 * M_PI

typedef struct vqe_settings {
  qubit * qr;
  cstate * cr;
  size_t NQUBITS;
  size_t NSHOTS;
  double prev_energy;
  double params[NPARAMS];
  double prev_params[NPARAMS];
  ptrdiff_t iter;
} vqe_settings;

#define NTERMS 5
#define NPAULIS MAX_NQUBITS * NTERMS
typedef struct hamiltonian {
  char paulis[NPAULIS];
  double coeffs[NTERMS];
  ptrdiff_t term_start_idx;
} hamiltonian;

extern hamiltonian h2_hamil;

void init_hf_state(qubit * qr, int num_spin_orbitals);

cq_status ansatz(const size_t NQUBITS,
                 qubit * qr,
                 const size_t NMEASURE,
                 cstate * cr,
                 void * kernpar,
                 pqkern_map * reg);

static double get_term_expectation(int * histogram,
                                   int num_bins,
                                   const size_t NMEASURE,
                                   const size_t NSHOTS,
                                   double coeff);

void init_vqe_settings(vqe_settings * settings,
                       qubit * qr,
                       cstate * cr,
                       const size_t NQUBITS,
                       const size_t NSHOTS);

void init_vqe_params(double * params, ptrdiff_t num_params);

double vqe_iter(qubit * qr,
                cstate * cr,
                const size_t NQUBITS,
                const size_t NSHOTS,
                const double * x);

double vqe_iter_nlopt(unsigned int n,
                      const double * x,
                      double * grad,
                      void * f_data);

double vqe_optimize(qubit * qr,
                    cstate * cr,
                    const size_t NQUBITS,
                    const size_t NMEASURE,
                    const size_t NSHOTS);

#endif
