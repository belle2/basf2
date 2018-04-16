/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Minakshi Nayak                                              *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <analysis/VariableManager/Manager.h>

#include <cmath>


namespace Belle2 {
  namespace Variable {

    double legacyGoodKs(const Particle* KS)
    {
      double p = particleP(KS);
      double fl = particleDRho(KS);
      double dphi = acos(((particleDX(KS) * particlePx(KS)) + (particleDY(KS) * particlePy(KS))) / (fl * sqrt(particlePx(KS) * particlePx(
                           KS) + particlePy(KS) * particlePy(KS))));
      double dr = std::min(abs(trackD0(KS->getDaughter(0))), abs(trackD0(KS->getDaughter(1))));
      double zdist = v0DaughterZ0Diff(KS);

      bool low = p < 0.5 && abs(zdist) < 0.8 && dr > 0.05 && dphi < 0.3;
      bool mid = p < 1.5 && p > 0.5 && abs(zdist) < 1.8 && dr > 0.03 && dphi < 0.1 && fl > .08;
      bool high = p > 1.5 && abs(zdist) < 2.4 && dr > 0.02 && dphi < 0.03 && fl > .22;


      if (low || mid || high) {
        return 1.0;
      } else return 0.0;
    }

    VARIABLE_GROUP("legacyGoodKs Variable");
    REGISTER_VARIABLE("legacyGoodKs", legacyGoodKs,
                      "[Legacy] GoodKs Returns 1.0 if a Kshort candidate passes the Belle algorithm: a momentum-binned selection including requirements on impact parameter of, and angle between the daughter pions as well as separation from the vertex and flight distance in the transverse plane");


  }
}
