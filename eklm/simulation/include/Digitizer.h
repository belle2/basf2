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
#include <eklm/geometry/TransformData.h>
#include <eklm/dataobjects/EKLMStepHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct DigitizationParams.
     * @brief Digitization parameters.
     *
     * @var DigitizationParams::ADCSamplingTime
     * ADC conversion time, ns.
     *
     * @var DigitizationParams::nDigitizations
     * Number of digitizations (points) in one sample.
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
     * @var DigitizationParams::firstPhotonlightSpeed
     * Speed of light in fiber, cm/ns.
     *
     * @var DigitizationParams::attenuationLength
     * Attenuation length in fiber, cm.
     *
     * @var DigitizationParams::expCoefficient
     * Exponent coefficient for a single photoelectron shape.
     *
     * @var DigitizationParams::meanSiPMNoise
     * Mean for SiPM backgrouns. If zero or negative no backgrounds are added.
     *
     * @var DigitizationParams::enableConstBkg
     * Enable background in fitting.
     *
     * @var DigitizationParams::debug
     * Debug mode (generates additional output files with histograms).
     */
    struct DigitizationParams {
      double ADCSamplingTime;
      int nDigitizations;
      double nPEperMeV;
      double minCosTheta;
      double mirrorReflectiveIndex;
      double scintillatorDeExcitationTime;
      double fiberDeExcitationTime;
      double firstPhotonlightSpeed;
      double attenuationLength;
      double expCoefficient;
      double meanSiPMNoise;
      bool enableConstBkg;
      bool debug;
    };

    /**
     * Digitize EKLMSimHits to get EKLM StripHits.
     * @details
     * Usually called by eklmDigitizerModule.
     */
    class Digitizer {

    public:

      /**
       * Constructor.
       */
      Digitizer(struct EKLM::TransformData* transf,
                struct EKLM::DigitizationParams* digPar);

      /**
       * Destructor.
       */
      ~Digitizer();

      /**
       * Read hits from the store, sort sim hits and fill m_HitStripMap.
       */
      void readAndSortStepHits();

      /**
       * Create SimHits from StepHits using boost:graph mechanism.
       */
      void makeSimHits();

      /**
       * Read hits from the store, sort sim hits and fill m_HitStripMap.
       */
      void readAndSortSimHits();

      /**
       * Merge hits from the same strip. Create EKLMDigits.
       */
      void mergeSimHitsToStripHits(double);

    private:

      /** Transformation data. */
      struct EKLM::TransformData* m_transf;

      /** Parameters. */
      struct EKLM::DigitizationParams* m_digPar;

      /** Map for EKLMStepHit sorting according sensitive volumes. */
      std::map<int, std::vector<EKLMStepHit*> > m_stepHitVolumeMap;

      /** Map for hits sorting according strip name. */
      std::map<int, std::vector<EKLMSimHit*> > m_HitStripMap;

      /** Vector of EKLMDigits. */
      std::vector<EKLMDigit*> m_HitVector;

      /** Sim hits vector. */
      std::vector<EKLMSimHit*> m_simHitsVector;

      /** SimHit storage initialization. */
      StoreArray<EKLMSimHit> m_simHitsArray;

      /** StripHit storage initialization. */
      StoreArray<EKLMDigit> m_stripHitsArray;

    };

  }

}

#endif

