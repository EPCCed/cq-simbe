#include <stdlib.h>
#include <stdio.h>
#include "qft.h"
#include "cq.h"

#include "nlopt.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

#define NLAYERS 1
#define MAX_NQUBITS 4
#define MAX_NQUBITS 2
#define NTERMS 15
#define NTERMS 5
#define NPAULIS MAX_NQUBITS * NTERMS
#define NPARAMS NLAYERS * MAX_NQUBITS * 2

double params[NPARAMS];

char paulis[NPAULIS] = {
'I', 'I',
'I', 'Z',
'Z', 'I',
'Z', 'Z',	
'X', 'X'
//  'I', 'I', 'I', 'I',
//  'Z', 'I', 'I', 'I',
//  'I', 'Z', 'I', 'I',
//  'Z', 'Z', 'I', 'I',
//  'Y', 'X', 'X', 'Y',
//  'Y', 'Y', 'X', 'X',
//  'X', 'X', 'Y', 'Y',
//  'X', 'Y', 'Y', 'X',
//  'I', 'I', 'Z', 'I',
//  'Z', 'I', 'Z', 'I',
//  'I', 'I', 'I', 'Z',
//  'Z', 'I', 'I', 'Z',
//  'I', 'Z', 'Z', 'I',
//  'I', 'Z', 'I', 'Z',
//  'I', 'I', 'Z', 'Z',
};

//double fci_for_big = -1.136189454088;

double coeffs[NTERMS] = {
	-1.052373245772859,
        0.39793742484318045,
        -0.39793742484318045,
        -0.01128010425623538,
        0.18093119978423156
//  -0.0996338794137,
//  0.1711054512372,
//  0.1711054512372,
//  0.16859349595532,
//  0.04533062254573,
//  -0.04533062254573,
//  -0.04533062254573,
//  0.04533062254573,
//  -0.222509142366,
//  0.12051027989546,
//  -0.222509142366,
//  0.1658409024411,
//  0.1658409024411,
//  0.12051027989546,
//  0.17432077259242
};

ptrdiff_t term_start_idx = 0;
ptrdiff_t iter = 0;

static cq_status ansatz(
		const size_t NQUBITS,
		qubit * qr,
		cstate * cr,
		qkern_map * reg) {
  CQ_REGISTER_KERNEL(reg)

  set_qureg(qr, 0, NQUBITS);
  // HF init state
  paulix(&qr[0]);
  paulix(&qr[1]);
  for (ptrdiff_t i = 0; i < NLAYERS; ++i) {
    for (ptrdiff_t j = 0; j < NQUBITS; ++j) {
      ptrdiff_t param_idx = j + i * (NQUBITS * 2);
      //printf("params: %f %f\n", params[param_idx], params[param_idx + 1]);
      roty(&qr[j], params[param_idx]);
      rotz(&qr[j], params[param_idx + 1]);

      if (j < NQUBITS - 1) {
        int control = j;
	int target = j + 1;
	cpaulix(&qr[control], &qr[target]);
      }

      if (i == NLAYERS - 1) {
	char pauli = paulis[term_start_idx + j];
	//printf("Pauli: %c\n", pauli);
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

static double get_term_expectation(int *histogram, int num_bins,
		const size_t NMEASURE, const size_t NSHOTS, double coeff) {
  double term_expectation = 0.0;

  //printf("term exp: %f\n", term_expectation);
  for (ptrdiff_t i = 0; i < num_bins; ++i) {
    int num_ones = count_ones(i, NMEASURE);
    //printf("i: %d count: %d\n", i, histogram[i]);
    double sign = -1.0;
    if (num_ones % 2 == 0) sign = 1.0;

    // num of the given count(results) / NSHOTS
    // cr as binary, to int
    // int as idx in histogram
    term_expectation += sign * ((double)histogram[i] / (double)NSHOTS);
    //printf("i: %d term exp: %f\n", i, term_expectation);
    histogram[i] = 0;
  }
  term_expectation *= coeff;
  return term_expectation;
}

static int result_to_int(cstate *cr, int num_qubits) {
  int result = 0;
  int base = 1;
  for (ptrdiff_t i = num_qubits - 1; i >= 0; --i) {
    if (cr[i] == -1) continue;
    //printf("CR: %d\n", cr[i]);
    int digit = cr[i] * base;
    result += digit;
    base *= 2;
  }
  //printf("bin2int: %d\n", result);
  return result;
}

#define MAX_BINS 1 << 10

static double vqe_iter(qubit * qr, cstate * cr, size_t NQUBITS, size_t NSHOTS) {
  int histogram[MAX_BINS] = {0};
  ptrdiff_t num_bins = (ptrdiff_t)1 << NQUBITS;
  double expectation = 0.0;
  const size_t NMEASURE = NQUBITS;
  term_start_idx = 0;

  for (ptrdiff_t i = 0; i < NTERMS; ++i) {
    ptrdiff_t paulis_start = i * NQUBITS;
    // FOR NSHOTS

    //ansatz(cr, qr, num_layers, num_qubits, params, &paulis[paulis_start]);
    // offload?
    //init_creg(NMEASURE * NSHOTS, -1, cr);

    sm_qrun(ansatz, qr, NQUBITS, cr, NMEASURE, NSHOTS);

    for (ptrdiff_t j = 0; j < NSHOTS; ++j) {
      ptrdiff_t cr_start = j * NQUBITS;
      //printf("CR_START %ld\n", cr_start);

      //printf("GOT HERE! %d\n", result_to_int(&cr[cr_start], NQUBITS));
      //exit(42);
      ++histogram[result_to_int(&cr[cr_start], NQUBITS)];
      //printf("GOT HERE! %ld\n", j);
    }

    double coeff = coeffs[i];
    //printf("coeff! %f\n", coeff);
    expectation += get_term_expectation(histogram, num_bins, NMEASURE, NSHOTS, coeff);
    term_start_idx += NQUBITS;
  }
  printf("Iter: %td -- Expectation: %f\n", iter, expectation);
  ++iter;
  return expectation;
}

typedef struct vqe_settings {
  qubit * qr;
  cstate * cr;
  size_t NQUBITS;
  size_t NSHOTS;
} vqe_settings;

double prev_energy = 0.0;
double prev_params[NPARAMS] = {0};
double vqe_iter_nlopt(unsigned int n, const double *x, double *grad, void *f_data) {
  vqe_settings *settings = (vqe_settings *)(f_data);

  double energy = vqe_iter(settings->qr, settings->cr, settings->NQUBITS, settings->NSHOTS);
  if (!grad) return energy;

  // estimate gradient if using gradient-based optimizer
  for (ptrdiff_t i = 0; i < NPARAMS; ++i) {
    grad[i] = (energy - prev_energy) / (x[i] - prev_params[i]);
    prev_params[i] = x[i];
  }
  prev_energy = energy;

  return energy;
}

void init_vqe_params(double *params, ptrdiff_t num_params) {
  double min_val = 0.0;
  double max_val = 2.0 * M_PI;
  int min = 0;
  int max = 100;
  double shift = max_val / (double)max;
  srand(102141);

  for (ptrdiff_t i = 0; i < NPARAMS; ++i) {
    int random_num = rand() % (max - min + 1) + min;
    params[i] = (double)random_num * shift;
  }
}

void init_vqe_settings(vqe_settings *settings, qubit * qr, cstate * cr,
		const size_t NQUBITS, const size_t NSHOTS) {
  settings->qr = qr;
  settings->cr = cr;
  settings->NQUBITS = NQUBITS;
  settings->NSHOTS = NSHOTS;
}

int main (void)
{

  const double TRUE_ENERGY = -1.85728;
  const size_t NQUBITS = MAX_NQUBITS;
  const size_t NSHOTS = 1024;
  const size_t NMEASURE = NQUBITS;

  init_vqe_params(params, NPARAMS);
  //init_vqe_params(prev_params, NPARAMS);

  cq_init(0);

  qubit * qr = NULL;
  alloc_qureg(&qr, NQUBITS);

  cstate * cr;
  cr = malloc(NMEASURE * NSHOTS * sizeof(cstate));
  init_creg(NMEASURE * NSHOTS, -1, cr);

  register_qkern(ansatz);
 
  // gradient-free optimizers
  nlopt_opt opt = nlopt_create(NLOPT_LN_COBYLA, NPARAMS);
  //nlopt_opt opt = nlopt_create(NLOPT_LN_SBPLX, NPARAMS);
  
  // gradient-based optimizers
  //nlopt_opt opt = nlopt_create(NLOPT_LD_LBFGS, NPARAMS);
  //nlopt_opt opt = nlopt_create(NLOPT_LD_SLSQP, NPARAMS);
  
  nlopt_set_maxeval(opt, 200);
  double ftol = 0.0001;
  nlopt_set_ftol_rel(opt, ftol);

  vqe_settings settings;
  init_vqe_settings(&settings, qr, cr, NQUBITS, NSHOTS);

  nlopt_result res = nlopt_set_min_objective(opt, vqe_iter_nlopt, (void *)&settings);
  nlopt_set_lower_bounds1(opt, 0.0);
  nlopt_set_upper_bounds1(opt, 2.0 * M_PI);

  double best_expectation = 0.0;
  res = nlopt_optimize(opt, params, &best_expectation);

  printf("The final expectation: %f\n", best_expectation);

  nlopt_destroy(opt);
  free_qureg(&qr);
  free(cr);

  cq_finalise(0);

  return 0;
}
