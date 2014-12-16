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

      /**
       * Get Drift length (cm).
       * @parm tdcCount              TDC count (ns).
       * @parm wireID                Encoded sense wire ID.
       * @parm timeOfFlightEstimator Time of flight (ns).
       * @parm leftRight             left/right flag.
      *  @param z                    z-position on the wire (cm).
       * @param alpha                Track incident angle in r-phi plane (rad).
       * @param theta                Track incident angle in s-z plane (=polar angle) (rad).
       */

      float getDriftLength(unsigned short tdcCount,
                           const WireID& wireID        = WireID(),
                           float timeOfFlightEstimator = 0,
                           bool leftRight = false,
                           float z = 0,
                           float alpha = 0,
                           float = static_cast<float>(TMath::Pi() / 2.));

      /**
       * Get position resolution^2 (cm^2) corresponding to drift length
       * from getDriftLength of this class.
       * @parm  driftLength  Drift length (cm).
       * @parm  wireID       Encoded sense wire ID.
       * @parm  leftRight    Left/right flag.
      *  @param z            z-position on the wire (cm).
       * @param alpha        Track incident angle in r-phi plane (rad).
       * @param theta        Track incident angle in s-z plane (=polar angle) (rad).
       * @return Uncertainty on the drift length.
       */
      float getDriftLengthResolution(float driftLength,
                                     const WireID& wireID = WireID(),
                                     bool leftRight = false,
                                     float z = 0,
                                     float alpha = 0,
                                     float = static_cast<float>(TMath::Pi() / 2.));

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
