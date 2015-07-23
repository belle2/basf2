#include <ecl/digitization/algorithms.h>

double Belle2::ECL::DspSamplingArray(int templateSignalNBins, double time, double templateSignalBinLen, const double* templateFun)
{
  int i, inext;
  double s1, s2;
  i = time / templateSignalBinLen;

  if (i < 0 || (i + 1) >= templateSignalNBins)
    return 0;

  s1 = templateFun[i];

  inext = i + 1;
  if (i < templateSignalNBins) {
    s2 = templateFun[inext];
  } else {
    s2 = 0;
  }
  return s1 + (s2 - s1) * (time / templateSignalBinLen - i);
}
