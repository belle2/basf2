/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/core/utilities.h>
#include <hlt/softwaretrigger/calculations/utilities.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TRandom.h>
#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    double getThetaOfNegativeTrackWithMaximumRhoInEvent()
    {
      StoreObjPtr<ParticleList> pionParticles("pi+:HLT");

      if (pionParticles.isValid()) {
        const Particle* trackWithMaximumRho = getElementWithMaximumRho<Particle>(pionParticles);
        if (trackWithMaximumRho) {
          if (trackWithMaximumRho->getCharge() == -1) {
            return trackWithMaximumRho->getMomentum().Theta();
          } else {
            const Particle* trackWithSecondMaximumRho = getElementWithMaximumRhoBelow<Particle>(pionParticles,
                                                        getRho(trackWithMaximumRho));

            if (trackWithSecondMaximumRho and trackWithSecondMaximumRho->getCharge() == -1) {
              return trackWithSecondMaximumRho->getMomentum().Theta();
            }
          }
        }
        return -1;
      } else {
        B2FATAL("You are using a list of pre scales although the pion list is not present! This is currently not possible.");
      }
    }

    bool makePreScale(const unsigned int& preScaleFactor)
    {
      // A prescale factor of one is always true...
      if (preScaleFactor == 1) {
        return true;
        // ... and a prescale factor of 0 is always false...
      } else if (preScaleFactor == 0) {
        return false;
      } else {
        // All other cases are a bit more interesting
        // We do this by drawing a random number between 0 and m_preScaleFactor - 1 and comparing it to 0.
        // The probability to get back a true result is then given by 1/m_preScaleFactor.
        const unsigned int randomNumber = gRandom->Integer(preScaleFactor);
        return randomNumber == 0;
      }
    }

    bool makePreScale(const std::vector<unsigned int>& preScaleFactors)
    {
      if (preScaleFactors.empty()) {
        return false;
      }

      const unsigned long factorSize = preScaleFactors.size();

      if (factorSize == 1) {
        return makePreScale(preScaleFactors.front());
      } else {
        const double& theta = getThetaOfNegativeTrackWithMaximumRhoInEvent();
        const double& intervalSize = TMath::Pi() / factorSize;

        if (theta < 0) {
          B2WARNING("Theta is negative, something went truly wrong here");
          return false;
        }

        const unsigned int index = static_cast<unsigned int>(std::floor(theta / intervalSize));

        B2ASSERT("Index (" << index << ") is larger than pre scale factor array (" << factorSize  << ").", index < factorSize);

        return makePreScale(preScaleFactors[index]);
      }
    }
  }
}