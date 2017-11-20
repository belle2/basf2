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
       *  @param alpha                   Track incident angle in r-phi plane.
       *  @param theta                   Track incident angle in s-z plane (=polar angle).
       *  @param adcCount                ADC count.
       *
       *  @return  Best estimation of closest distance between the track and the wire.
       */
      virtual double getDriftLength(unsigned short tdcCount     = 0,
                                    const WireID& wireID        = WireID(),
                                    double timeOfFlightEstimator = 0.,
                                    bool ambiguityDiscrimiator  = false,
                                    double z = 0,
                                    double alpha = 0,
                                    double theta = static_cast<double>(TMath::Pi() / 2.),
                                    unsigned short adcCount = 0) = 0;

      /**
       * Get Drift time.
       * @param tdcCount              TDC count (ns).
       * @param wireID                Encoded sense wire ID.
       * @param timeOfFlightEstimator Time of flight (ns).
       * @param z                     z-position on the wire (cm).
       * @param adcCount              ADC count.
       * @return Drift time (ns)
       */
      virtual double getDriftTime(unsigned short tdcCount,
                                  const WireID& wireID,
                                  double timeOfFlightEstimator,
                                  double z,
                                  unsigned short adcCount) = 0;


      /** Uncertainty corresponding to drift length from getDriftLength of this class.
       *
       *  @param driftLength             Output of the getDriftLength function.
       *  @param ambiguityDiscriminator  Information to resolve left/right ambiguity.
       *  @param z                       z-position for determining the in-wire-propagation time.
       *  @param alpha                   Track incident angle in r-phi plane.
       *  @param theta                   Track incident angle in s-z plane (=polar angle).
       */
      virtual double getDriftLengthResolution(double driftLength          = 0.,
                                              const WireID& wireID       = WireID(),
                                              bool ambiguityDiscrimiator = false,
                                              double z = 0,
                                              double alpha = 0,
                                              double theta = static_cast<double>(TMath::Pi() / 2.)) = 0;
    };
  }
}
#endif /* DRIFTTIMETRANSLATORBASE_H_ */
