/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Tracking headers. */
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/dataobjects/EventT0.h>

/* Basf2 headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>
#include <mdst/dataobjects/Track.h>

namespace Belle2 {
  namespace Variable {

    //HELPERS

    //! @returns a pointer to the related RecoTrack dataobject
    RecoTrack* getRecoTrack(const Particle* particle)
    {
      const Track* track = particle->getTrack();
      if (!track)
        return nullptr;
      return track->getRelatedTo<RecoTrack>();
    }


    // CHARGE-RELATED VARIABLES
    double seedCharge(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getChargeSeed();
    }

    //! @returns the quality indicator of the first MVA of the Flip&Refit
    double firstFlipQualityIndicator(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getFlipQualityIndicator();
    }
    //! @returns the quality indicator of the second MVA of the Flip&Refit
    double secondFlipQualityIndicator(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->get2ndFlipQualityIndicator();
    }


    // TIME-RELATED VARIABLES

    //! @returns the see time of the RecoTrack
    double seedTime(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getTimeSeed();
    }

    //! @returns the number of SVDhits on the RecoTrack outgoing arm
    double outgoingArmNSVDHits(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getNSVDHitsOfOutgoingArm();
    }

    //! @returns the RecoTrack outgoingArmTime
    double outgoingArmTime(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getOutgoingArmTime();
    }

    //! @returns the RecoTrack outgoingArmTimeError
    double outgoingArmTimeError(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getOutgoingArmTimeError();
    }


    //! @returns the number of SVDhits on the  RecoTrack ingoing arm
    double ingoingArmNSVDHits(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getNSVDHitsOfIngoingArm();
    }

    //! @returns the RecoTrack ingoingArmTime
    double ingoingArmTime(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getIngoingArmTime();
    }

    //! @returns the RecoTrack ingoingArmTimeError
    double ingoingArmTimeError(const Particle* particle)
    {
      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
        return Const::doubleNaN;
      return recoTrack->getIngoingArmTimeError();
    }

    //! @returns the SVDEventT0
    double SVDEventT0(const Particle*)
    {

      StoreObjPtr<EventT0> m_evtT0;
      if (m_evtT0.isValid()) {
        const auto bestSVDHypo = m_evtT0->getBestSVDTemporaryEventT0();
        if (bestSVDHypo) {
          return bestSVDHypo->eventT0;
        }
      }
      return Const::doubleNaN;
    }


    VARIABLE_GROUP("Tracking Validation");

    REGISTER_VARIABLE("seedCharge", seedCharge,
                      "[RecoTrack] Returns the charge of the seed");

    REGISTER_VARIABLE("firstFlipQualityIndicator", firstFlipQualityIndicator,
                      "[RecoTrack] Returns the quality indicator of the first MVA of the Flip&Refit")
    REGISTER_VARIABLE("secondFlipQualityIndicator", secondFlipQualityIndicator,
                      "[RecoTrack] Returns the quality indicator of the second MVA of the Flip&Refit")

    REGISTER_VARIABLE("seedTime", seedTime,
                      "[RecoTrack] Returns the time of the seed");
    REGISTER_VARIABLE("outgoingArmNSVDHits", outgoingArmNSVDHits,
                      "[RecoTrack] Returns the number of SVD hits on the outgoing arm");
    REGISTER_VARIABLE("outgoingArmTime", outgoingArmTime,
                      "[RecoTrack] Returns the outgoing arm time");
    REGISTER_VARIABLE("outgoingArmTimeError", outgoingArmTimeError,
                      "[RecoTrack] Returns the outgoing arm time");
    REGISTER_VARIABLE("ingoingArmNSVDHits", ingoingArmNSVDHits,
                      "[RecoTrack] Returns the number of SVD hits on the ingoing arm");
    REGISTER_VARIABLE("ingoingArmTime", ingoingArmTime,
                      "[RecoTrack] Returns the ingoing arm time");
    REGISTER_VARIABLE("ingoingArmTimeError", ingoingArmTimeError,
                      "[RecoTrack] Returns the ingoing arm time");
    REGISTER_VARIABLE("SVDEventT0", SVDEventT0,
                      "[Event] Returns the SVDEventT0");
  }
}
