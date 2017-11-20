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
#include <cdc/geometry/CDCGeoControlPar.h>

#include <framework/dataobjects/EventT0.h>

namespace Belle2 {
  namespace CDC {
    /** Translator mirroring the realistic Digitization. */
    class RealisticTDCCountTranslator : public TDCCountTranslatorBase {

    public:
      /** Constructor, with the additional information, if propagation in the wire shall be considered. */
      explicit RealisticTDCCountTranslator(bool useInWirePropagationDelay = false);

      /** Destructor. */
      ~RealisticTDCCountTranslator() {};

      /**
       * If trigger jitter was simulated, in every event one has to give an estimate of the effect.
       * To reproduce the old behaviour, the other extracted event T0s are deleted before.
       * */
      void setEventTime(double eventTime = 0) __attribute__((deprecated))
      {
        if (not m_eventTimeStoreObject.isValid()) {
          m_eventTimeStoreObject.create();
        }

        m_eventTimeStoreObject->clear();
        m_eventTimeStoreObject->addEventT0(eventTime, 0, Const::CDC);
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
                            unsigned short adcCount = 0) override;

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
                          unsigned short adcCount) override;

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
                                      double = static_cast<double>(TMath::Pi() / 2.)) override;

    private:
      /**
       * Flag to activate the propagation delay of the sense wire.
       * true : activated, false : the propagation delay is not used.
       *
       */
      bool m_useInWirePropagationDelay;

      /**
       * Flag to distinguish betw. data and MC.
       */
      bool m_realData = false;

      /**
       * Event timing. The event time is fetched from the data store using this pointer.
       */
      StoreObjPtr<EventT0> m_eventTimeStoreObject;

      /**
       * Cached reference to CDC GeoControlPar object.
       */
      const CDCGeoControlPar& m_gcp;

      /**
       * Cached reference to CDC GeometryPar object.
       */
      const CDCGeometryPar& m_cdcp;

      //      /**
      //       * Hit timing offset value.
      //       */
      //      unsigned short m_tdcOffset;

      /**
       * Cached TDC bin width (ns).
       * N.B. The declaration should be after m_cdcp for proper initialization.
       */
      const double m_tdcBinWidth;
    };
  }
}
#endif /* RealisticTDCCountTranslator_H_ */
