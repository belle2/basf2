/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dbobjects/EKLMChannelData.h>
#include <eklm/dbobjects/EKLMDigitizationParameters.h>
#include <klm/simulation/ScintillatorFirmware.h>

namespace Belle2 {

  namespace KLM {

    /**
     * Digitize EKLMSim2Hits to get EKLM StripHits.
     */
    class ScintillatorSimulator : public EKLMHitMCTime {

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
      ScintillatorSimulator(const EKLMDigitizationParameters* digPar,
                            ScintillatorFirmware* fitter,
                            double digitizationInitialTime,
                            bool debug);

      /**
       * Copy constructor (disabled).
       */
      ScintillatorSimulator(const ScintillatorSimulator&) = delete;

      /**
       * Operator = (disabled).
       */
      ScintillatorSimulator& operator=(const ScintillatorSimulator&) = delete;

      /**
       * Destructor.
       */
      ~ScintillatorSimulator();

      /**
       * Simulate BKLM strip.
       * @param[in] firstHit First hit in this strip.
       * @param[in] end      End of hit range.
       */
      void simulate(std::multimap<int, BKLMSimHit*>::iterator& firstHit,
                    std::multimap<int, BKLMSimHit*>::iterator& end);

      /**
       * Simulate EKLM strip.
       * @param[in] firstHit First hit in this strip.
       * @param[in] end      End of hit range.
       */
      void simulate(std::multimap<int, EKLMSimHit*>::iterator& firstHit,
                    std::multimap<int, EKLMSimHit*>::iterator& end);

      /**
       * Get fit data.
       */
      KLMScintillatorFirmwareFitResult* getFPGAFit();

      /**
       * Get fit status.
       * @return Status of the fit.
       */
      enum ScintillatorFirmwareFitStatus getFitStatus() const;

      /**
       * Get number of photoelectrons (fit result).
       */
      double getNPE();

      /**
       * Get generated number of photoelectrons.
       */
      int getGeneratedNPE();

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
      ScintillatorFirmware* m_fitter;

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
      enum ScintillatorFirmwareFitStatus m_FPGAStat;

      /** FPGA fit data. */
      KLMScintillatorFirmwareFitResult m_FPGAFit;

      /** Number of photoelectrons (generated). */
      int m_npe;

      /** Name of the strip. */
      std::string m_stripName;

      /** Pedestal. */
      double m_Pedestal;

      /** Photoelectron amplitude. */
      double m_PhotoelectronAmplitude;

      /** Threshold. */
      int m_Threshold;

      /**
       * Reallocate photoelectron buffers.
       * @param[in] size New size of buffers.
       */
      void reallocPhotoElectronBuffers(int size);

      /**
       * Prepare simulation.
       */
      void prepareSimulation();

      /**
       *  Perform common simulation stage.
       */
      void performSimulation();

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
