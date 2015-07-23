#include <vector>

namespace Belle2 {
  namespace ECL {
    double Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1);
    double ShaperDSP(double time);
    double ShaperDSP_F(double time, float* parameters);
    double DspSamplingArray(int templateSignalNBins, double time, double templateSignalBinLen, const double* templateFun);
    std::vector<double>* createDefSampledSignalWF();
  }
}
