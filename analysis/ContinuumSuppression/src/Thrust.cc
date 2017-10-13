#include <analysis/ContinuumSuppression/Thrust.h>

using namespace Belle2;

TVector3 Thrust::calculateThrust(const std::vector<TVector3>& momenta)
{

  decltype(momenta.begin()) p;
  decltype(momenta.begin()) q;
  decltype(momenta.begin()) loopcount;

  const auto begin = momenta.begin();
  const auto end = momenta.end();

  double sump = 0;
  for (p = begin; p != end; p++)
    sump += (*p).Mag();


  TVector3 Axis;

  // Thrust and thrust vectors

  double Thru = 0;
  for (p = begin; p != end; p++) {
    TVector3 rvec(*p);
    if (rvec.z() <= 0.0) rvec = -rvec;

    double s = rvec.Mag();
    if (s != 0.0) rvec *= (1 / s);

    for (loopcount = begin; loopcount != end; loopcount++) {
      TVector3 rprev(rvec);
      rvec = TVector3(); // clear

      for (q = begin; q != end; q++) {
        const TVector3 qvec(*q);
        rvec += (qvec.Dot(rprev) >= 0) ? qvec : - qvec;
      }

      for (q = begin; q != end; q++) {
        const TVector3 qvec(*q);
        if (qvec.Dot(rvec) * qvec.Dot(rprev) < 0) break;
      }

      if (q == end) break;
    }

    double ttmp = 0.0;
    for (q = begin; q != end; q++) {
      const TVector3 qvec = *q;
      ttmp += std::fabs(qvec.Dot(rvec));
    }
    ttmp /= (sump * rvec.Mag());
    rvec *= 1 / rvec.Mag();
    if (ttmp > Thru) {
      Thru = ttmp;
      Axis = rvec;
    }
  }
  Axis *= Thru;
  return Axis;
}
