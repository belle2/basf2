#include <analysis/ContinuumSuppression/FoxWolfram.h>

using namespace Belle2;


FoxWolfram::FoxWolfram(const std::vector<TVector3>& momenta) : sum {0, 0, 0, 0, 0}
{
  decltype(momenta.begin()) p;
  decltype(momenta.begin()) q;

  const auto begin = momenta.begin();
  const auto end = momenta.end();


  for (p = begin; p != end; p++) {
    const TVector3 pvec = (*p);
    double pmag = pvec.Mag();
    for (q = p; q != end; q++) {
      const TVector3 qvec = (*q);
      double mag = pmag * qvec.Mag();
      double costh = pvec.Dot(qvec) / mag;
      if (p != q) mag *= 2;
      add(mag, costh);
    }
  }
}


void FoxWolfram::add(const double mag, const double costh)
{
  double cost2 = costh * costh;

  sum[0] += mag;
  sum[1] += mag * costh;
  sum[2] += mag * (1.5 * cost2 - 0.5);
  sum[3] += mag * costh * (2.5 * cost2 - 1.5);
  sum[4] += mag * (4.375 * cost2 * cost2 - 3.75 * cost2 + 0.375);
}