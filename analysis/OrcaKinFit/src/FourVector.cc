/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * See https://github.com/tferber/OrcaKinfit, forked from                 *
 * https://github.com/iLCSoft/MarlinKinfit                                *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (torben.ferber@desy.de) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef MARLIN_USE_ROOT

#include "analysis/OrcaKInFit/FourVector.h"

#include <cmath>

#undef NDEBUG
#include <cassert>

#include <TRandom3.h>

namespace Belle2 {
  namespace OrcaKinFit {

    static TRandom* rnd = 0;

    FourVector& FourVector::boost(const FourVector& P)
    {
      // See CERNLIB U101 for a description

      double pP = -(p * P.p);
      double e = getE();
      double M = P.getM();

      E = (e * P.getE() - pP) / M;
      p = p - ((pP / (P.getE() + M) - e) / M) * P.p;

      return *this;
    }

    void FourVector::decayto(FourVector& d1, FourVector& d2) const
    {
      // Let this particle decay isotropically into 4-vectors d1 and d2;
      // d1 and d2 must have definite mass at beginning
      using std::abs;
      using std::sqrt;
      using std::pow;

      double M2 = getM2();
      double M  = getM();;
      double m1 = d1.getM();
      double m2 = d2.getM();

      double randoms[2];
      if (rnd == 0) rnd = new TRandom3();
      rnd->RndmArray(2, randoms);


      assert(m1 + m2 <= M);

      double pstar = 0.5 * sqrt(abs((M2 - pow(m1 + m2, 2)) * (M2 - pow(m1 - m2, 2)))) / M;
      double phistar = 2 * M_PI * randoms[0];
      double costhetastar = 2 * randoms[1] - 1;
      double sinthetastar = sqrt(abs(1 - costhetastar * costhetastar));
      double E1 = sqrt(m1 * m1 + pstar * pstar);
      double E2 = sqrt(m2 * m2 + pstar * pstar);


      d1 = FourVector(E1, pstar * sinthetastar * cos(phistar),
                      pstar * sinthetastar * sin(phistar),
                      pstar * costhetastar);
      d2 = FourVector(E2, -pstar * sinthetastar * cos(phistar),
                      -pstar * sinthetastar * sin(phistar),
                      -pstar * costhetastar);

      d1.boost(*this);
      d2.boost(*this);

    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // MARLIN_USE_ROOT
