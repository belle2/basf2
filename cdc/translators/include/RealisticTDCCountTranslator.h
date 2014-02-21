/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REALISTICTDCCOUNTTRANSLATOR_H
#define REALISTICTDCCOUNTTRANSLATOR_H

#include <cdc/dataobjects/TDCCountTranslatorBase.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TVector3.h>

namespace Belle2 {
  namespace CDC {
    /** Translator mirroring the realistic Digitization. */
    class RealisticTDCCountTranslator : public TDCCountTranslatorBase {
    public:
      /** Constructor, with the additional information, if propagation in the wire shall be considered. */
      RealisticTDCCountTranslator(bool useInWirePropagationDelay = false);

      /** Destructor. */
      ~RealisticTDCCountTranslator() {};

      /** If trigger jitter was simulated, in every event one has to give an estimate of the effect. */
      void setEventTime(short eventTime = 0) {
        m_eventTime = eventTime;
      }

      /** */
      float getDriftLength(unsigned short tdcCount,
                           const WireID& wireID        = WireID(),
                           float timeOfFlightEstimator = 0,
                           bool leftRight = false,
                           float z = 0, float = static_cast<float>(TMath::Pi() / 2.));

      /** */
      float getDriftLength(unsigned short tdcCount,
                           const TVector3& posOnWire,
                           const TVector3& posOnTrack,
                           const TVector3& momentum,
                           const WireID& wireID        = WireID(),
                           float timeOfFlightEstimator = 0);


      /** Uncertainty corresponding to drift length from getDriftLength of this class.
       *
       *  @return Uncertainty on the drift length.
       */
      float getDriftLengthResolution(float driftLength,
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

      /**
       * CDC Geometry parameters.
       */
      CDCGeometryPar& m_cdcp;

      /**
       * Hit timing offset value.
       */
      unsigned short m_tdcOffset;

      /**
       * TDC bin width (1nsec).
       */

      float m_tdcBinWidth;
    };
  }
}
#endif /* RealisticTDCCountTranslator_H_ */
