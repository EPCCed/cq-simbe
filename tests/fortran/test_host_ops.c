#include "unity.h"
// #include <stdlib.h>

// extern int ___cq_MOD_cq_init(int);
// extern int ___cq_MOD_cq_finalise(int);
extern int cq_init(int);
extern int cq_finalise(int);

void test_smth()
{
    //    const size_t NQUBITS = 5;
    //    const size_t NMEASURE = NQUBITS;
    //    const size_t NSHOTS = 1;
    //    const short CR_INIT_VAL = -1;
    //    short* cr;
    //    ___cq_MOD_cq_init(0);
    //    ___cq_MOD_cq_finalise(0);
    cq_init(0);
    cq_finalise(0);
}
