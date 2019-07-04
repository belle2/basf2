/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Minakshi Nayak, Sam Cunliffe                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/BelleVariables.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/variables/Variables.h>
#include <analysis/variables/VertexVariables.h>
#include <analysis/variables/ECLVariables.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/variables/ParameterVariables.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace Variable {

    double goodBelleKshort(const Particle* KS)
    {
      // check input
      if (KS->getNDaughters() != 2) {
        B2WARNING("goodBelleKshort is only defined for a particle with two daughters");
        return 0.0;
      }
      const Particle* d0 = KS->getDaughter(0);
      const Particle* d1 = KS->getDaughter(1);
      if ((d0->getCharge() == 0) || (d1->getCharge() == 0)) {
        B2WARNING("goodBelleKshort is only defined for a particle with charged daughters");
        return 0.0;
      }
      if (abs(KS->getPDGCode()) != 310)
        B2WARNING("goodBelleKshort is being applied to a candidate with PDG " << KS->getPDGCode());

      // Belle selection
      double p = particleP(KS);
      double fl = particleDRho(KS);
      double dphi = acos(((particleDX(KS) * particlePx(KS)) + (particleDY(KS) * particlePy(KS))) / (fl * sqrt(particlePx(KS) * particlePx(
                           KS) + particlePy(KS) * particlePy(KS))));
      double dr = std::min(abs(trackD0(d0)), abs(trackD0(d1)));
      double zdist = v0DaughterZ0Diff(KS);

      bool low = p < 0.5 && abs(zdist) < 0.8 && dr > 0.05 && dphi < 0.3;
      bool mid = p < 1.5 && p > 0.5 && abs(zdist) < 1.8 && dr > 0.03 && dphi < 0.1 && fl > .08;
      bool high = p > 1.5 && abs(zdist) < 2.4 && dr > 0.02 && dphi < 0.03 && fl > .22;

      if (low || mid || high) {
        return 1.0;
      } else
        return 0.0;
    }

    bool isGoodBelleGamma(int region, double energy)
    {
      bool goodGammaRegion1, goodGammaRegion2, goodGammaRegion3;
      goodGammaRegion1 = region == 1 && energy > 0.100;
      goodGammaRegion2 = region == 2 && energy > 0.050;
      goodGammaRegion3 = region == 3 && energy > 0.150;

      return goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3;
    }

    double goodBelleGamma(const Particle* particle)
    {
      double energy = eclClusterE(particle);
      int region = eclClusterDetectionRegion(particle);

      return (double) isGoodBelleGamma(region, energy);
    }

    VARIABLE_GROUP("Belle Variables");

    REGISTER_VARIABLE("goodBelleKshort", goodBelleKshort,
                      "[Legacy] GoodKs Returns 1.0 if a Kshort candidate passes the Belle algorithm:"
                      "a momentum-binned selection including requirements on impact parameter of, and"
                      "angle between the daughter pions as well as separation from the vertex and flight distance in the transverse plane");

    REGISTER_VARIABLE("goodBelleGamma", goodBelleGamma, R"DOC(
[Legacy] Returns 1.0 if photon candidate passes simple region dependent
energy selection for Belle data and MC (50/100/150 MeV).
)DOC");

    // this is defined in ECLVariables.{h,cc}
    REGISTER_VARIABLE("clusterBelleQuality", eclClusterDeltaL, R"DOC(
[Legacy] Returns ECL cluster's quality indicating a good cluster in GSIM (stored in deltaL of ECL cluster object).
Belle analysis typically used clusters with quality == 0 in their :math:`E_{\text{extra ECL}}` (Belle only).
)DOC");
  }
}
