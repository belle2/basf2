/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMFIBERANDELECTRONICS_H
#define EKLMFIBERANDELECTRONICS_H

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSim2Hit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/Digitizer.h>
#include <eklm/simulation/FPGAFitter.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Digitize EKLMSim2Hits to get EKLM StripHits.
     */
    class FiberAndElectronics {

    public:

      /**
       * Constructor.
       */
      FiberAndElectronics(std::pair <int, std::vector<EKLMSim2Hit*> >,
                          EKLM::GeometryData* geoDat,
                          struct EKLM::DigitizationParams* digPar);

      /**
       * Destructor.
       */
      ~FiberAndElectronics();

      /**
       * Process.
       */
      void processEntry();

      /**
       * Calculate StripHit times (at the end of the strip),
       * @param[in]  stripLen    Length of strip.
       * @param[in]  distSipm    Distance to SiPM.
       * @param[in]  nPE         Number of photoelectrons.
       * @param[in]  timeShift   Time of the SimHit.
       * @param[in]  isReflected If the hit is direct or reflected.
       * @param[in]  digPar      Digitization parameters.
       * @param[out] hist        Histogram.
       * @return Vector of hit times.
       */
      void fillAmplitude(double stripLen, double distSiPM, int nPE,
                         double timeShift, bool isReflected,
                         struct DigitizationParams* digPar, float* hist);

      /**
       * Get fit results.
       */
      struct FPGAFitParams* getFitResults();

      /**
       * Get fit status.
       * @return Status of the fit.
       */
      enum FPGAFitStatus getFitStatus() const;

      /**
       * Get number of photoelectrons (fit result).
       */
      double getNPE();

      /**
       * Get generated number of photoelectrons.
       */
      int getGeneratedNPE();

    private:

      /** Geometry data. */
      EKLM::GeometryData* m_geoDat;

      /** Parameters. */
      struct EKLM::DigitizationParams* m_digPar;

      /** Stands for nDigitizations*ADCSamplingTime. */
      double m_histRange;

      /** Analog amplitude (direct). */
      float* m_amplitudeDirect;

      /** Analog amplitude (reflected). */
      float* m_amplitudeReflected;

      /** Analog amplitude. */
      float* m_amplitude;

      /** Digital amplitude. */
      int* m_ADCAmplitude;

      /** Digital fit result. */
      float* m_ADCFit;

      /** FPGA fit status. */
      enum FPGAFitStatus m_FPGAStat;

      /** FPGA fit results. */
      struct FPGAFitParams m_FPGAParams;

      /** Number of photoelectrons (generated). */
      int m_npe;

      /** Pointer to vector if the SimHits. */
      std::vector<EKLMSim2Hit*> m_vectorHits;

      /** Name of the strip. */
      std::string m_stripName;

      /**
       * Add random noise to the signal (amplitude-dependend).
       */
      void addRandomSiPMNoise();

      /**
       * Simulate ADC (create digital signal from analog),
       */
      void simulateADC();

      /**
       * Debug output (signal and fit result histograms).
       */
      void debugOutput();

    };

  }

}

#endif

