/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>
#include <klm/dataobjects/KLMSimHit.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMScintillatorFEEData.h>
#include <klm/simulation/ScintillatorFirmware.h>
#include <klm/time/KLMTime.h>

namespace Belle2 {

  namespace KLM {

    /**
     * Digitize EKLMSim2Hits to get EKLM StripHits.
     */
    class ScintillatorSimulator {

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
      ScintillatorSimulator(const KLMScintillatorDigitizationParameters* digPar,
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
       * Simulate a strip.
       * @param[in] firstHit First hit in this strip.
       * @param[in] end      End of hit range.
       */
      void simulate(
        const std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator& firstHit,
        const std::multimap<KLMChannelNumber, const KLMSimHit*>::iterator& end);

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
      double getNPhotoelectrons();

      /**
       * Get generated number of photoelectrons.
       */
      int getNGeneratedPhotoelectrons();

      /**
       * Get total energy deposited in the strip (sum over ssimulation hits).
       */
      double getEnergy();

      /**
       * Set FEE data.
       */
      void setFEEData(const KLMScintillatorFEEData* FEEData);

      /**
       * Generate photoelectrons.
       * @param[in] stripLen    Strip length.
       * @param[in] distSiPM    Distance from hit to SiPM.
       * @param[in] nPhotons    Number of photons to be simulated.
       * @param[in] timeShift   Time of hit.
       * @param[in] isReflected Whether the hits are reflected or not.
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

      /**
       * Get MC time.
       * @return MC time.
       */
      float getMCTime() const
      {
        return m_MCTime;
      }

      /**
       * Get SiPM MC time.
       * @return SiPM MC yime.
       */
      float getSiPMMCTime() const
      {
        return m_SiPMMCTime;
      }

    private:

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

      /** Time. */
      const KLMTime* m_Time;

      /** Parameters. */
      const KLMScintillatorDigitizationParameters* m_DigPar;

      /** Fitter. */
      ScintillatorFirmware* m_fitter;

      /** Initial digitization time. */
      double m_DigitizationInitialTime;

      /** Debug mode (generates additional output files with histograms). */
      bool m_Debug;

      /** Time range, (number of digitizations) * (ADC sampling time). */
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

      /** Total energy deposited in the strip. */
      double m_Energy;

      /** Name of the strip. */
      std::string m_stripName;

      /** Pedestal. */
      double m_Pedestal;

      /** Photoelectron amplitude. */
      double m_PhotoelectronAmplitude;

      /** Threshold. */
      int m_Threshold;

      /** MC time. */
      float m_MCTime;

      /** MC time at SiPM. */
      float m_SiPMMCTime;

    };

  }

}
