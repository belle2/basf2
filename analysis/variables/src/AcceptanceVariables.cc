/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/AcceptanceVariables.h>
#include <analysis/VariableManager/Manager.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    //Theta Acceptance

    bool thetaInCDCAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 17. && theta < 150.) {
        return true;
      } else return false;
    }

    bool thetaInTOPAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 31. && theta < 128.) {
        return true;
      } else return false;
    }

    bool thetaInARICHAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta > 14. && theta < 30.) {
        return true;
      } else return false;
    }

    int thetaInECLAcceptance(const Particle* particle)
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

    bool thetaInBECLAcceptance(const Particle* particle)
    {
      double acceptance = thetaInECLAcceptance(particle);
      if (acceptance == 2) {
        return true;
      } else return false;
    }

    bool thetaInEECLAcceptance(const Particle* particle)
    {
      double acceptance = thetaInECLAcceptance(particle);
      if (acceptance == 1 || acceptance == 3) {
        return true;
      } else return false;
    }

    int thetaInKLMAcceptance(const Particle* particle)
    {
      double theta = particle->get4Vector().Theta() * 180. / TMath::Pi();
      if (theta < 18.)  return 0;
      if (theta < 37.)  return 1; //forward endcap
      if (theta < 47.)  return 2; //forward overlap
      if (theta < 122.) return 3; //barrel
      if (theta < 130.) return 4; //backward overlap
      if (theta < 155.) return 5; //backward endcap
      else return 0;
    }

    bool thetaInBKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 2 || acceptance == 3 || acceptance == 4) {
        return true;
      } else return false;
    }

    bool thetaInEKLMAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance != 0 && acceptance != 3) {
        return true;
      } else return false;
    }

    bool thetaInKLMOverlapAcceptance(const Particle* particle)
    {
      double acceptance = thetaInKLMAcceptance(particle);
      if (acceptance == 2 || acceptance == 4) {
        return true;
      } else return false;
    }

    //Pt Acceptance

    bool ptInTOPAcceptance(const Particle* particle)
    {
      if (particle->getCharge() == 0) return 1;
      double pt = particle->get4Vector().Pt();
      if (pt > 0.27)  return true;
      else  return false;
    }

    bool ptInBECLAcceptance(const Particle* particle)
    {
      if (particle->getCharge() == 0) return 1;
      double pt = particle->get4Vector().Pt();
      if (pt > 0.28)  return true;
      else  return false;
    }

    bool ptInBKLMAcceptance(const Particle* particle)
    {
      if (particle->getCharge() == 0) return 1;
      double pt = particle->get4Vector().Pt();
      if (pt > 0.6)  return true;
      else  return false;
    }

    //Combined Acceptance

    bool inCDCAcceptance(const Particle* particle)
    {
      return thetaInCDCAcceptance(particle);
    }

    bool inTOPAcceptance(const Particle* particle)
    {
      return (thetaInTOPAcceptance(particle) && ptInTOPAcceptance(particle));
    }

    bool inARICHAcceptance(const Particle* particle)
    {
      return thetaInARICHAcceptance(particle);
    }

    bool inECLAcceptance(const Particle* particle)
    {
      return (thetaInEECLAcceptance(particle) || (thetaInBECLAcceptance(particle) && ptInBECLAcceptance(particle)));
    }

    bool inKLMAcceptance(const Particle* particle)
    {
      return (thetaInEKLMAcceptance(particle) || (thetaInBKLMAcceptance(particle) && ptInBKLMAcceptance(particle)));
    }

    // ---

    VARIABLE_GROUP("Acceptance");

    REGISTER_VARIABLE("thetaInCDCAcceptance",   thetaInCDCAcceptance, R"DOC(
Returns true if particle is within CDC angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`17^\circ < \theta < 150^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInTOPAcceptance",   thetaInTOPAcceptance, R"DOC(
Returns true if particle is within TOP angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`31^\circ < \theta < 128^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInARICHAcceptance", thetaInARICHAcceptance, R"DOC(
Returns true if particle is within ARICH angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`14^\circ < \theta < 30^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInECLAcceptance",   thetaInECLAcceptance, R"DOC(
Checks if particle is within ECL angular acceptance. 
This variable checks if the particle polar angle :math:`\theta` is within certain ranges.
Return values and the corresponding :math:`\theta` ranges are the following:

* 0: Outside of ECL acceptance, :math:`\theta < 12.4^\circ` or :math:`\theta < 155.1^\circ`.
* 1: Forward ECL,  :math:`12.4^\circ < \theta < 31.4^\circ`; 
* 2: Barrel ECL,   :math:`32.2^\circ < \theta < 128.7^\circ`; 
* 3: Backward ECL, :math:`130.7^\circ < \theta < 155.1^\circ`.

The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInBECLAcceptance",  thetaInBECLAcceptance, R"DOC(
Returns true if particle is within Barrel ECL angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`32.2^\circ < \theta < 128.7^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInEECLAcceptance",  thetaInEECLAcceptance, R"DOC(
Returns true if particle is within Endcap ECL angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`12.4^\circ < \theta < 31.4^\circ` 
or :math:`130.7^\circ < \theta < 155.1^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInKLMAcceptance",   thetaInKLMAcceptance,  R"DOC(
Checks if particle is within KLM angular acceptance. 
This variable checks if the particle polar angle :math:`\theta` is within certain ranges.
Return values and the corresponding :math:`\theta` ranges are the following:

* 0: Outside of KLM acceptance, :math:`\theta < 18^\circ` or :math:`\theta < 155^\circ`.
* 1: Forward endcap,   :math:`18^\circ < \theta < 37^\circ`; 
* 2: Forward overlap,  :math:`37^\circ < \theta < 47^\circ`; 
* 3: Barrel,           :math:`47^\circ < \theta < 122^\circ`; 
* 4: Backward overlap, :math:`122^\circ < \theta < 130^\circ`; 
* 5: Backward endcap,  :math:`130^\circ < \theta < 155^\circ`.

The polar angle is computed using only the initial particle momentum.
)DOC");

    REGISTER_VARIABLE("thetaInBKLMAcceptance",  thetaInBKLMAcceptance, R"DOC(
Returns true if particle is within Barrel KLM angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`37^\circ < \theta < 130^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInEKLMAcceptance",  thetaInEKLMAcceptance, R"DOC(
Returns true if particle is within Endcap KLM angular acceptance, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`18^\circ < \theta < 47^\circ` or :math:`122^\circ < \theta < 155^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");
    REGISTER_VARIABLE("thetaInKLMOverlapAcceptance",  thetaInKLMOverlapAcceptance, R"DOC(
Returns true if particle is within the angular region where KLM barrel and endcaps overlap, false otherwise.
This variable checks if the particle polar angle :math:`\theta` is within the range :math:`37^\circ < \theta < 47^\circ` or :math:`122^\circ < \theta < 130^\circ`.
The polar angle is computed using only the initial particle momentum.
)DOC");

    REGISTER_VARIABLE("ptInTOPAcceptance",   ptInTOPAcceptance,  "Returns true if particle transverse momentum :math:`p_t` is within TOP acceptance, :math:`p_t > 0.27` GeV, false otherwise.");
    REGISTER_VARIABLE("ptInBECLAcceptance",  ptInBECLAcceptance, "Returns true if particle transverse momentum :math:`p_t` is within Barrel ECL acceptance, :math:`p_t > 0.28` GeV, false otherwise.");
    REGISTER_VARIABLE("ptInBKLMAcceptance",  ptInBKLMAcceptance, "Returns true if particle transverse momentum :math:`p_t` is within Barrel KLM acceptance, :math:`p_t > 0.6` GeV, false otherwise.");

    REGISTER_VARIABLE("inCDCAcceptance",   inCDCAcceptance, R"DOC(
Returns true if particle is within CDC geometrical and kinematical acceptance, false otherwise. 
This variable is an alias for :b2:var:`thetaInCDCAcceptance`.
)DOC");
    REGISTER_VARIABLE("inTOPAcceptance",   inTOPAcceptance, R"DOC(
Returns true if particle is within TOP geometrical and kinematical acceptance, false otherwise. 
This variable is a combination of :b2:var:`thetaInTOPAcceptance` and :b2:var:`ptInTOPAcceptance`.
)DOC");
    REGISTER_VARIABLE("inARICHAcceptance", inARICHAcceptance, R"DOC(
Returns true if particle is within ARICH geometrical and kinematical acceptance, false otherwise.
This variable is an alias for :b2:var:`thetaInARICHAcceptance`.
)DOC");
    REGISTER_VARIABLE("inECLAcceptance",   inECLAcceptance, R"DOC(
Returns true if particle is within ECL geometrical and kinematical acceptance, false otherwise.");
This variable is a combination of :b2:var:`thetaInEECLAcceptance`, :b2:var:`thetaInBECLAcceptance` and :b2:var:`ptInBECLAcceptance`.
)DOC");
    REGISTER_VARIABLE("inKLMAcceptance",   inKLMAcceptance, R"DOC(
Returns true if particle is within KLM geometrical and kinematical acceptance, false otherwise.
This variable is a combination of :b2:var:`thetaInEKLMAcceptance`, :b2:var:`thetaInBKLMAcceptance` and :b2:var:`ptInBKLMAcceptance`.
)DOC");

  }
}
