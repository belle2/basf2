/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIMPLEDRIFTTIMETRANSLATOR_H
#define SIMPLEDRIFTTIMETRANSLATOR_H

#include <cdc/dataobjects/DriftTimeTranslatorBase.h>
#include <TVector3.h>

namespace Belle2 {
  namespace CDC {
    /** Translator mirroring the simple Digitization. */
    class SimpleDriftTimeTranslator : public DriftTimeTranslatorBase {
    public:
      /** Constructor, with the additional information, if propagation in the wire shall be considered. */
      SimpleDriftTimeTranslator(bool useInWirePropagationDelay = false) :
        m_useInWirePropagationDelay(useInWirePropagationDelay), m_eventTime(0) {
      }

      /** Destructor. */
      ~SimpleDriftTimeTranslator() {};

      /** If trigger jitter was simulated, in every event one has to give an estimate of the effect. */
      void setEventTime(short eventTime = 0) {
        m_eventTime = eventTime;
      }

      /** */
      float getDriftLength(short driftTime,
                           const WireID& wireID = WireID(),
                           unsigned short timeOfFlightEstimator = 0,
                           bool = false,
                           float z = 0, float = static_cast<float>(TMath::Pi() / 2.));

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
      bool m_useInWirePropagationDelay;
      TVector3 m_backWirePos;

      short m_eventTime;
    };
  }
}
#endif /* SIMPLEDRIFTTIMETRANSLATOR_H_ */
