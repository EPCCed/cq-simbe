#include "vqe.h"
#include <stdio.h>

#include "nlopt.h"

void init_hf_state(qubit * qr, int num_spin_orbitals) {
  for (ptrdiff_t i = 0; i < num_spin_orbitals; ++i) {
    paulix(&qr[i]);
  }
}

cq_status ansatz(const size_t NQUBITS, qubit * qr, const size_t NMEASURE, cstate * cr, void * kernpar, pqkern_map * reg) {
  CQ_REGISTER_KERNEL(reg)

  double * params = (double *)kernpar;

  HANDLE_CQ_ERROR(set_qureg(qr, 0, NQUBITS));
  // HF init state. For H2 we set NQUBITS/2 to 1
  init_hf_state(qr, NQUBITS / 2);

  // preparing RYCZ ansatz
  for (ptrdiff_t i = 0; i < NLAYERS; ++i) {
    for (ptrdiff_t j = 0; j < NQUBITS; ++j) {
      ptrdiff_t param_idx = j + i * (NQUBITS * 2);
      roty(&qr[j], params[param_idx]);

      if (j < NQUBITS - 1) {
        int control = j;
	int target = j + 1;
	cpauliz(&qr[control], &qr[target]);
      }

      // Changing basis to get expectation value
      if (i == NLAYERS - 1) {
	char pauli = h2_hamil.paulis[h2_hamil.term_start_idx + j];
        if (pauli == 'X') {
          hadamard(&qr[j]);
          measure_qubit(&qr[j], &cr[j]);
        
        } else if (pauli == 'Y') {
          rotx(&qr[j], M_PI / 2.0);
          measure_qubit(&qr[j], &cr[j]);
        
        } else if (pauli == 'Z') {
          measure_qubit(&qr[j], &cr[j]);
        
        } else {
          // On I -- do nothing
        }
      }
    }
  }
  return CQ_SUCCESS;
}

static int count_ones(int dec_idx, const size_t NMEASURE) {
  int num_ones = 0;
  for (ptrdiff_t i = NMEASURE - 1; i >= 0; --i) {
    if (dec_idx % 2 == 1) ++num_ones;
    dec_idx >>= 1;
  }
  return num_ones;
}

double get_term_expectation(int *histogram, int num_bins,
		const size_t NMEASURE, const size_t NSHOTS, double coeff) {
  double term_expectation = 0.0;

  for (ptrdiff_t i = 0; i < num_bins; ++i) {
    int num_ones = count_ones(i, NMEASURE);
    double sign = -1.0;
    if (num_ones % 2 == 0) sign = 1.0;

    term_expectation += sign * ((double)histogram[i] / (double)NSHOTS);
    histogram[i] = 0;
  }
  term_expectation *= coeff;
  return term_expectation;
}

#define EXAMPLE_SEED 102141
void init_vqe_params(double *params, ptrdiff_t num_params) {
  const double min_val = PARAM_MIN;
  const double max_val = PARAM_MAX;
  const int min = 0;
  const int max = 100;
  const double shift = max_val / (double)max;
  srand(EXAMPLE_SEED);

  for (ptrdiff_t i = 0; i < NPARAMS; ++i) {
    const int random_num = rand() % (max - min + 1) + min;
    params[i] = (double)random_num * shift;
  }
}
#undef EXAMPLE_SEED

void init_vqe_settings(vqe_settings *settings, qubit * qr, cstate * cr,
		const size_t NQUBITS, const size_t NSHOTS) {
  settings->qr = qr;
  settings->cr = cr;
  settings->NQUBITS = NQUBITS;
  settings->NSHOTS = NSHOTS;
}

static int result_to_int(cstate *cr, int num_qubits) {
  int result = 0;
  int base = 1;
  for (ptrdiff_t i = num_qubits - 1; i >= 0; --i) {
    if (cr[i] == -1) continue;
    const int digit = cr[i] * base;
    result += digit;
    base *= 2;
  }
  return result;
}

#define MAX_BINS 1 << MAX_NQUBITS

double vqe_iter(qubit * qr, cstate * cr, const size_t NQUBITS, const size_t NSHOTS, const double * x) {
  int histogram[MAX_BINS] = {0};
  const ptrdiff_t num_bins = (ptrdiff_t)1 << NQUBITS;
  double expectation = 0.0;
  const size_t NMEASURE = NQUBITS;
  h2_hamil.term_start_idx = 0;

  for (ptrdiff_t i = 0; i < NTERMS; ++i) {
    ptrdiff_t paulis_start = i * NQUBITS;

    smp_qrun(ansatz, x, NPARAMS * sizeof(double), qr, NQUBITS, cr, NMEASURE, NSHOTS);

    for (ptrdiff_t j = 0; j < NSHOTS; ++j) {
      ptrdiff_t cr_start = j * NQUBITS;
      ++histogram[result_to_int(&cr[cr_start], NQUBITS)];
    }

    double coeff = h2_hamil.coeffs[i];
    expectation += get_term_expectation(histogram, num_bins, NMEASURE, NSHOTS, coeff);
    h2_hamil.term_start_idx += NQUBITS;
  }
  return expectation;
}

#undef MAX_BINS

double vqe_iter_nlopt(unsigned int n, const double *x, double *grad, void *f_data) {
  vqe_settings *settings = (vqe_settings *)(f_data);

  double energy = vqe_iter(settings->qr, settings->cr, settings->NQUBITS, settings->NSHOTS, x);

  printf("Iter: %td -- Expectation: %f\n", settings->iter, energy);
  ++settings->iter;

  if (!grad) return energy;

  // estimate gradient if using gradient-based optimizer
  for (ptrdiff_t i = 0; i < NPARAMS; ++i) {
    grad[i] = (energy - settings->prev_energy) / (x[i] - settings->prev_params[i]);
    settings->prev_params[i] = x[i];
  }
  settings->prev_energy = energy;

  return energy;
}

double vqe_optimize(qubit * qr, cstate * cr, const size_t NQUBITS,
		    const size_t NMEASURE, const size_t NSHOTS) {
  vqe_settings settings = {0};
  init_vqe_settings(&settings, qr, cr, NQUBITS, NSHOTS);

  init_vqe_params(settings.params, NPARAMS);

  // gradient-free optimizers
  nlopt_opt opt = nlopt_create(NLOPT_LN_COBYLA, NPARAMS);
  //nlopt_opt opt = nlopt_create(NLOPT_LN_SBPLX, NPARAMS);
  
  // gradient-based optimizers
  //nlopt_opt opt = nlopt_create(NLOPT_LD_LBFGS, NPARAMS);
  //nlopt_opt opt = nlopt_create(NLOPT_LD_SLSQP, NPARAMS);
  
  nlopt_set_maxeval(opt, 200);
  double ftol = 0.0001;
  nlopt_set_ftol_rel(opt, ftol);

  nlopt_result res = nlopt_set_min_objective(opt, vqe_iter_nlopt, (void *)&settings);
  nlopt_set_lower_bounds1(opt, PARAM_MIN);
  nlopt_set_upper_bounds1(opt, PARAM_MAX);

  double best_expectation = 0.0;
  res = nlopt_optimize(opt, settings.params, &best_expectation);

  printf("The final expectation: %f\n", best_expectation);

  nlopt_destroy(opt);
  return best_expectation;
}
