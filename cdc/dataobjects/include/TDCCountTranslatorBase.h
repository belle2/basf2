/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DRIFTTIMETRANSLATORBASE_H
#define DRIFTTIMETRANSLATORBASE_H

#include <cdc/dataobjects/WireID.h>

#include <TVector3.h>
#include <TMath.h>

namespace Belle2 {
  namespace CDC {
    /** Base class for translation of Drift Time into Drift Length.
     *
     *  Specific implementations are implemented in the cdc library.
     */
    class TDCCountTranslatorBase {
    public:
      /** Constructor. */
      TDCCountTranslatorBase() {}

      /** Destructor. */
      virtual ~TDCCountTranslatorBase() {}

      /** Function for getting a drift length estimation.
       *
       *  @param tdcCount                This is the "drift time" saved in the CDCHit. Actually it can be the sum of various effects
       *                                 such as trigger time jitter, propagation time of the signal in the wire of the CDC etc.
       *                                 Actual translators should however get the appropriate drift length including the specified effects.
       *  @param wireID                  Object to identify hit wire.
       *  @param timeOfFlightEstimator   This is an estimator for the time, that lies between the event time/ time of creation
       *                                 of the particle and the time at which the ionisation happened, which is the starting point
       *                                 for the calculation of the drift length/actual drift time.
       *  @param ambiguityDiscriminator  Information to resolve left/right ambiguity.
       *  @param z                       z-position for determining the in-wire-propagation time.
       *  @param theta                   Angle under which the particle moves through the drift-cell in r-phi.
       *
       *  @return  Best estimation of closest distance between the track and the wire.
       */
      virtual float getDriftLength(unsigned short tdcCount     = 0,
                                   const WireID& wireID        = WireID(),
                                   float timeOfFlightEstimator = 0.,
                                   bool ambiguityDiscrimiator  = false,
                                   float z = 0, float theta = static_cast<float>(TMath::Pi() / 2.)) = 0;

      /** Function for realistic xt and sigma */
      virtual float getDriftLength(unsigned short tdcCount = 0,
                                   const TVector3& posWire = TVector3(),
                                   const TVector3& posTrack = TVector3(),
                                   const TVector3& momentum = TVector3(),
                                   const WireID& wireID = WireID(),
                                   float timeOfFlightEstimator = 0.) = 0;

      /** Uncertainty corresponding to drift length from getDriftLength of this class.
       *
       *  @param driftLength             Output of the getDriftLength function.
       *  @param ambiguityDiscriminator  Information to resolve left/right ambiguity.
       *  @param z                       z-position for determining the in-wire-propagation time.
       *  @param theta                   Angle under which the particle moves through the drift-cell in r-phi.
       */
      virtual float getDriftLengthResolution(float driftLength          = 0.,
                                             const WireID& wireID       = WireID(),
                                             bool ambiguityDiscrimiator = false,
                                             float z = 0, float theta = static_cast<float>(TMath::Pi() / 2.)) = 0;
    };
  }
}
#endif /* DRIFTTIMETRANSLATORBASE_H_ */
