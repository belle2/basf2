/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZER_H
#define EKLMDIGITIZER_H

/* C++ headers. */
#include <map>
#include <vector>

/* External headers. */
#include <CLHEP/Vector/ThreeVector.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMSim2Hit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/simulation/FPGAFitter.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct DigitizationParams.
     * @brief Digitization parameters.
     *
     * @var DigitizationParams::ADCRange
     * ADC range: 2**(resolution in bits).
     *
     * @var DigitizationParams::ADCSamplingTime
     * ADC conversion time, ns.
     *
     * @var DigitizationParams::nDigitizations
     * Number of digitizations (points) in one sample.
     *
     * @var DigitizationParams::ADCPedestal
     * ADC pedestal.
     *
     * @var DigitizationParams::ADCPEAmplitude
     * ADC photoelectron amplitude.
     *
     * @var DigitizationParams::ADCSaturation
     * ADC readout corresponding to saturation.
     *
     * @var DigitizationParams::nPEperMeV
     * Number of photoelectrons / 1 MeV.
     *
     * @var DigitizationParams::minCosTheta
     * Cosine of maximal light capture angle (by fiber).
     *
     * @var DigitizationParams::mirrorReflectiveIndex
     * Mirror reflective index.
     *
     * @var DigitizationParams::scintillatorDeExcitationTime
     * Scintillator deexcitation time, ns.
     *
     * @var DigitizationParams::fiberDeExcitationTime
     * Fiber deexcitation time, ns.
     *
     * @var DigitizationParams::fiberLightSpeed
     * Speed of light in fiber, cm/ns.
     *
     * @var DigitizationParams::attenuationLength
     * Attenuation length in fiber, cm.
     *
     * @var DigitizationParams::PEAttenuationFreq
     * Attenuation frequency of a single photoelectron pulse, ns^-1.
     *
     * @var DigitizationParams::meanSiPMNoise
     * Mean for SiPM backgrouns. If zero or negative no backgrounds are added.
     *
     * @var DigitizationParams::enableConstBkg
     * Enable background in fitting.
     *
     * @var DigitizationParams::timeResolution
     * Time resolution (of reconstructed time, not ADC).
     *
     * @var DigitizationParams::debug
     * Debug mode (generates additional output files with histograms).
     */
    struct DigitizationParams {
      double ADCRange;
      double ADCSamplingTime;
      int nDigitizations;
      double ADCPedestal;
      double ADCPEAmplitude;
      double ADCSaturation;
      double nPEperMeV;
      double minCosTheta;
      double mirrorReflectiveIndex;
      double scintillatorDeExcitationTime;
      double fiberDeExcitationTime;
      double fiberLightSpeed;
      double attenuationLength;
      double PEAttenuationFreq;
      double meanSiPMNoise;
      bool enableConstBkg;
      double timeResolution;
      bool debug;
    };

    /**
     * Set default digitization parameters.
     * @param[out] digPar Digitization parameters.
     */
    void setDefDigitizationParams(struct DigitizationParams* digPar);

    /**
     * Digitize EKLMSim2Hits to get EKLM StripHits.
     * @details
     * Usually called by eklmDigitizerModule.
     */
    class Digitizer {

    public:

      /**
       * Constructor.
       */
      Digitizer(struct EKLM::DigitizationParams* digPar);

      /**
       * Destructor.
       */
      ~Digitizer();

      /**
       * Read hits from the store, sort sim hits and fill m_HitStripMap.
       */
      void readAndSortSimHits();

      /**
       * Create EKLMSim2Hits from EKLMSimHits using boost:graph mechanism.
       */
      void makeSim2Hits();

      /**
       * Merge hits from the same strip. Create EKLMDigits.
       */
      void mergeSimHitsToStripHits(double threshold);

    private:

      /** Parameters. */
      struct EKLM::DigitizationParams* m_digPar;

      /** Geometry data. */
      const EKLM::GeometryData* m_GeoDat;

      /** FPGA fitter. */
      FPGAFitter m_fitter;

      /** Map for EKLMSimHit sorting according sensitive volumes. */
      std::multimap<int, EKLMSimHit*> m_simHitVolumeMap;

      /** Vector of EKLMDigits. */
      std::vector<EKLMDigit*> m_HitVector;

      /** Sim hits vector. */
      std::vector<EKLMSim2Hit*> m_simHitsVector;

      /** SimHit storage initialization. */
      StoreArray<EKLMSim2Hit> m_simHitsArray;

      /** StripHit storage initialization. */
      StoreArray<EKLMDigit> m_stripHitsArray;

    };

  }

}

#endif

