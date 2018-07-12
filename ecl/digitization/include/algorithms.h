#include <vector>

namespace Belle2 {
  namespace ECL {
    double Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1);
    double ShaperDSP(double time);
    double ShaperDSP(double time, const float*);
    double ShaperDSP(double time, const double*);
    double ShaperDSPofflineFit(double time, const double*, double scale);
    void shapeFitter(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31, int* fg32, int* fg33, int* y,
                     int* ttrig2, int* n16,  int* lar, int* ltr, int* lq, int* hi2);
  }
}
