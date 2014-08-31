/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLETDCCOUNTTRANSLATOR_H
#define SIMPLETDCCOUNTTRANSLATOR_H

#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <TVector3.h>

namespace Belle2 {
  namespace CDC {
    /** Translator mirroring the simple Digitization. */
    class SimpleTDCCountTranslator : public TDCCountTranslatorBase {
    public:
      /** Constructor, with the additional information, if propagation in the wire shall be considered. */
      SimpleTDCCountTranslator(bool useInWirePropagationDelay = false) :
        m_useInWirePropagationDelay(useInWirePropagationDelay), m_eventTime(0) {
      }

      /** Destructor. */
      ~SimpleTDCCountTranslator() {};

      /** If trigger jitter was simulated, in every event one has to give an estimate of the effect. */
      void setEventTime(short eventTime = 0) {
        m_eventTime = eventTime;
      }

      /** Returns drift length corresponding to input TDC-counts. */
      float getDriftLength(unsigned short tdcCount,
                           const WireID& wireID        = WireID(),
                           float timeOfFlightEstimator = 0,
                           bool = false,
                           float z = 0, float = static_cast<float>(TMath::Pi() / 2.));

      /**
        * Get Drift length.
        * @parm tdcCount TDC value in nsec.
        * @parm posOnWire Wire position of closest approach.
        * @parm posOnTrack position at closest approach.
        * @parm posOnTrack momentum at closest approach.
        * @parm wireID encoded sense wire ID.
        * @parm timeOfFlightEstimator Time of Flight in nsec.
        *
        */

      float getDriftLength(unsigned short tdcCount,
                           const TVector3& posOnWire,
                           const TVector3& posOnTrack,
                           const TVector3& momentum,
                           const WireID& wireID        = WireID(),
                           float timeOfFlightEstimator = 0);


      /** Uncertainty corresponding to drift length from getDriftLength of this class.
       *
       *  Currently in the simple digitization just a Gaussian smearing is used.
       *
       *  @return Uncertainty on the drift length, currently 0.001.
       */
      float getDriftLengthResolution(float,
                                     const WireID&,
                                     bool,
                                     float, float);

    private:
      /**
       * Flag to activate the propagation delay of the sense wire.
       * true : activated, false : the propagation delay is not used.
       *
       */
      bool m_useInWirePropagationDelay;

      /**
       * Wire position at the cdc backward endplate.
       */
      TVector3 m_backWirePos;

      /**
       * Event timing.
       * If this is not simulated, m_eventTime is set to be 0.
       */
      float m_eventTime;
    };
  }
}
#endif /* SimpleTDCCountTranslator_H_ */
