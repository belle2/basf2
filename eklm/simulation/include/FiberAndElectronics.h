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
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dbobjects/EKLMChannelData.h>
#include <eklm/dbobjects/EKLMDigitizationParameters.h>
#include <eklm/simulation/FPGAFitter.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * Digitize EKLMSim2Hits to get EKLM StripHits.
     */
    class FiberAndElectronics : public EKLMHitMCTime {

    public:

      /** Photoelectron data. */
      struct Photoelectron {
        int bin;          /**< Hit time bin in ADC output histogram */
        double expTime;   /**< exp(-m_DigPar->PEAttenuationFreq * (-time)) */
        bool isReflected; /**< Direct (false) or reflected (true). */
      };

      /**
       * Constructor.
       * @param[in] digPar                  Digitization parameters.
       * @param[in] digitizationInitialTime Initial digitization time.
       * @param[in] fitter                  Fitter.
       * @param[in] debug                   Use debug mode.
       */
      FiberAndElectronics(const EKLMDigitizationParameters* digPar,
                          FPGAFitter* fitter, double digitizationInitialTime,
                          bool debug);

      /**
       * Destructor.
       */
      ~FiberAndElectronics();

      /**
       * Process.
       */
      void processEntry();

      /**
       * Get fit data.
       */
      EKLMFPGAFit* getFPGAFit();

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

      /**
       * Set hit range.
       * @param[in] it  First hit in this strip.
       * @param[in] end End of hit range.
       */
      void setHitRange(std::multimap<int, EKLMSimHit*>::iterator& it,
                       std::multimap<int, EKLMSimHit*>::iterator& end);

      /**
       * Set channel data.
       */
      void setChannelData(const EKLMChannelData* channelData);

      /**
       * Generate photoelectrons.
       * @param[in]     stripLen    Strip length.
       * @param[in]     distSiPM    Distance from hit to SiPM.
       * @param[in]     nPE         Number of photons to be simulated.
       * @param[in]     timeShift   Time of hit.
       * @param[in]     isReflected Whether the hits are reflected or not.
       */
      void generatePhotoelectrons(double stripLen, double distSiPM,
                                  int nPhotons, double timeShift,
                                  bool isReflected);

      /**
       * Fill SiPM output.
       * @param[in,out] hist         Output histogram (signal is added to it).
       * @param[in]     useDirect    Use direct photons.
       * @param[in]     useReflected Use reflected photons.
       */
      void fillSiPMOutput(float* hist, bool useDirect, bool useReflected);

    private:

      /** Parameters. */
      const EKLMDigitizationParameters* m_DigPar;

      /** Fitter. */
      FPGAFitter* m_fitter;

      /** Initial digitization time. */
      double m_DigitizationInitialTime;

      /** Debug mode (generates additional output files with histograms). */
      bool m_Debug;

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

      /** Buffer for signal time dependence calculation. */
      double* m_SignalTimeDependence;

      /** Buffer for signal time dependence calculation. */
      double* m_SignalTimeDependenceDiff;

      /** Buffer for photoelectron data. */
      struct Photoelectron* m_Photoelectrons;

      /** Buffer for photoelectron indices. */
      int* m_PhotoelectronIndex;

      /** Buffer for photoelectron indices. */
      int* m_PhotoelectronIndex2;

      /** Size of photoelectron data buffer. */
      int m_PhotoelectronBufferSize;

      /** FPGA fit status. */
      enum FPGAFitStatus m_FPGAStat;

      /** FPGA fit data. */
      EKLMFPGAFit m_FPGAFit;

      /** Number of photoelectrons (generated). */
      int m_npe;

      /** First hit. */
      std::multimap<int, EKLMSimHit*>::iterator m_hit;

      /** End of hits. */
      std::multimap<int, EKLMSimHit*>::iterator m_hitEnd;

      /** Name of the strip. */
      std::string m_stripName;

      /** Channel data. */
      const EKLMChannelData* m_ChannelData;

      /**
       * Reallocate photoelectron buffers.
       * @param[in] size New size of buffers.
       */
      void reallocPhotoElectronBuffers(int size);

      /**
       * Sort photoelectrons.
       * @param[in] nPhotoelectrons Number of photoelectrons.
       * @return Pointer to index array.
       */
      int* sortPhotoelectrons(int nPhotoelectrons);

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

