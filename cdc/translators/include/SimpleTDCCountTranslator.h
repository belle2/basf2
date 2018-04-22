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

#include <framework/logging/Logger.h>
#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <TVector3.h>

namespace Belle2 {
  namespace CDC {
    /** Translator mirroring the simple Digitization. */
    class SimpleTDCCountTranslator : public TDCCountTranslatorBase {
    public:
      /** Constructor, with the additional information, if propagation in the wire shall be considered. */
      explicit SimpleTDCCountTranslator(bool useInWirePropagationDelay = false) :
        m_useInWirePropagationDelay(useInWirePropagationDelay), m_eventTime(0)
      {
        B2FATAL("This translator is disabled; please use realisticTDCCountTranslator instead.");
      }

      /** Destructor. */
      ~SimpleTDCCountTranslator() {};

      /** If trigger jitter was simulated, in every event one has to give an estimate of the effect. */
      void setEventTime(short eventTime = 0)
      {
        m_eventTime = eventTime;
      }

      /**
        * Get Drift length (cm).
        * @parm tdcCount              TDC count (ns).
        * @parm wireID                Encoded sense wire ID.
        * @parm timeOfFlightEstimator Time of Flight (ns).
        * @parm leftRight             left/right flag.
        * @parm z                     z-positin on the wire (cm).
        *
        */

      double getDriftLength(unsigned short tdcCount,
                            const WireID& wireID        = WireID(),
                            double timeOfFlightEstimator = 0,
                            bool leftRight = false,
                            double z = 0,
                            double = 0,
                            double = 0,
                            unsigned short = 0) override;

      /**
       * Get Drift time.
       * @param tdcCount              TDC count (ns).
       * @param wireID                Encoded sense wire ID.
       * @param timeOfFlightEstimator Time of flight (ns).
       * @param z                     z-position on the wire (cm).
       * @param adcCount              ADC count.
       * @return Drift time (ns)
       */
      double getDriftTime(unsigned short tdcCount,
                          const WireID& wireID,
                          double timeOfFlightEstimator,
                          double z,
                          unsigned short) override;

      /** Uncertainty corresponding to drift length from getDriftLength of this class.
       *
       *  Currently in the simple digitization just a Gaussian smearing is used.
       *
       *  @return Uncertainty on the drift length, currently 1.69e-4.
       */
      double getDriftLengthResolution(double,
                                      const WireID& = WireID(),
                                      bool = false,
                                      double = 0,
                                      double = 0,
                                      double = 0) override;

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
    };
  }
}
#endif /* SimpleTDCCountTranslator_H_ */
