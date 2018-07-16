/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Francesco Tenchini                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/AcceptanceVariables.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double thetaInCDCAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 17. && theta < 150.) {
        return 1;
      } else return 0;
    }

    double thetaInTOPAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 31. && theta < 128.) {
        return 1;
      } else return 0;
    }

    double thetaInARICHAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 14. && theta < 30.) {
        return 1;
      } else return 0;
    }

    double thetaInECLAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 12.4 && theta < 31.4) { //forward
        return 1;
      } else if (theta > 32.2 && theta < 128.7) { //barrel
        return 2;
      } else if (theta > 130.7 && theta < 155.1) { //backwards
        return 3;
      } else return 0;
    }

    double thetaInKLMAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta < 25.)  return 0;
      if (theta < 40.)  return 1; //forward
      if (theta < 129.) return 2; //barrel
      if (theta < 155.) return 3; //backwards
      else return 0;
    }

    double thetaInBKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 2) {
        return 1;
      } else return 0;
    }

    double thetaInEKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 1 || acceptance == 3) {
        return 1;
      } else return 0;
    }

    VARIABLE_GROUP("Acceptance");

    REGISTER_VARIABLE("thetaInCDCAcceptance",   thetaInCDCAcceptance, "Particle is within CDC angular acceptance.");
    REGISTER_VARIABLE("thetaInTOPAcceptance",   thetaInCDCAcceptance, "Particle is within TOP angular acceptance.");
    REGISTER_VARIABLE("thetaInARICHAcceptance", thetaInCDCAcceptance, "Particle is within ARICH angular acceptance.");
    REGISTER_VARIABLE("thetaInECLAcceptance",   thetaInCDCAcceptance,
                      "Particle is within ECL angular acceptance. 1: Forward; 2: Barrel; 3: Backwards.");
    REGISTER_VARIABLE("thetaInKLMAcceptance",   thetaInCDCAcceptance,
                      "Particle is within KLM angular acceptance. 1: Forward; 2: Barrel; 3: Backwards.");
    REGISTER_VARIABLE("thetaInBKLMAcceptance",  thetaInCDCAcceptance, "Particle is within Barrel KLM angular acceptance.");
    REGISTER_VARIABLE("thetaInEKLMAcceptance",  thetaInCDCAcceptance, "Particle is within Endcap KLM angular acceptance.");

  }
}
