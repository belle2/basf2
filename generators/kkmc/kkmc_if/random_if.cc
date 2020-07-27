#include <TRandom.h>
#include <stdio.h>

double SFMTgenrand_real3()
{
  return gRandom->Rndm();
}
extern "C" {
  void kkmc_rand_init_(unsigned long* seed)
  {
    float dum;
    printf("kkmc: kkmc_rand_init seed= %lu\n", *seed);
    //  init_SFMTgen_rand(*seed);
    dum = (float)SFMTgenrand_real3();
    printf("kkmc: kkmc_rand_init test random number= %f\n", dum);
    dum = (float)SFMTgenrand_real3();
    printf("kkmc: kkmc_rand_init test random number= %f\n", dum);
  }

  void kkmc_rand_real_(float* rnd)
  {
    do {
      *rnd = (float)SFMTgenrand_real3();
    } while (*rnd == 0.0);
  }

  void kkmc_rand_double_(double* rnd)
  {
    do {
      *rnd = SFMTgenrand_real3();
    } while (*rnd == 0.0);
  }

  void kkmc_ranmar_(float* rvec, int* lenv)
  {
    int i;
    /*  printf("kkmc_ranmar; %d\n",*lenv); */
    for (i = 0; i < *lenv; i++) {
      do {
        rvec[i] = (float)SFMTgenrand_real3();
      } while (rvec[i] == 0.0);
      /*    printf("kkmc_ranmar; %f\n",rvec[i]); */
    }
  }

  void kkmc_ranlux_(float* rvec, int* lenv)
  {
    /*  printf("kkmc_ranlux; %d\n",*lenv); */
    kkmc_ranmar_(rvec, lenv);
    /*    printf("kkmc_ranlux; %f\n",rvec[i]); */
  }

}
