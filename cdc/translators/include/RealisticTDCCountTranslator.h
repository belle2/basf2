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
      explicit RealisticTDCCountTranslator(bool useInWirePropagationDelay = false);

      /** Destructor. */
      ~RealisticTDCCountTranslator() {};

      /** If trigger jitter was simulated, in every event one has to give an estimate of the effect. */
      void setEventTime(double eventTime = 0)
      {
        m_eventTime = eventTime;
      }

      /**
       * Get Drift length.
       * @param tdcCount              TDC count (ns).
       * @param wireID                Encoded sense wire ID.
       * @param timeOfFlightEstimator Time of flight (ns).
       * @param leftRight             left/right flag.
       * @param z                     z-position on the wire (cm).
       * @param alpha                 Track incident angle in r-phi plane (rad).
       * @param theta                 Track incident angle in s-z plane (=polar angle) (rad).
       * @param adcCount              ADC count.
       * @return Drift length (cm) if drift time >= 0; v*(drift time) (i.e. negative value) otherwise, where v is set to one (cm/ns) for simplicity.
       */
      double getDriftLength(unsigned short tdcCount,
                            const WireID& wireID        = WireID(),
                            double timeOfFlightEstimator = 0,
                            bool leftRight = false,
                            double z = 0,
                            double alpha = 0,
                            double theta = static_cast<double>(TMath::Pi() / 2.),
                            unsigned short adcCount = 0);

      /**
       * Get position resolution^2 corresponding to the drift length
       * from getDriftLength of this class.
       * @param  driftLength  Drift length (cm).
       * @param  wireID       Encoded sense wire ID.
       * @param  leftRight    Left/right flag.
       * @param  z            z-position on the wire (cm).
       * @param  alpha        Track incident angle in r-phi plane (rad).
       * @param  theta        Track incident angle in s-z plane (=polar angle) (rad).
       * @return Uncertainty^2 (cm^2) on the drift length.
       */
      double getDriftLengthResolution(double driftLength,
                                      const WireID& wireID = WireID(),
                                      bool leftRight = false,
                                      double z = 0,
                                      double alpha = 0,
                                      double = static_cast<double>(TMath::Pi() / 2.));

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
      double m_eventTime;

      /**
       * Reference to CDC GeometryPar object.
       */
      const CDCGeometryPar& m_cdcp;

      //      /**
      //       * Hit timing offset value.
      //       */
      //      unsigned short m_tdcOffset;

      /**
       * TDC bin width (ns).
       * N.B. The declaration should be after m_cdcp for proper initialization.
       */
      const double m_tdcBinWidth;
    };
  }
}
#endif /* RealisticTDCCountTranslator_H_ */
