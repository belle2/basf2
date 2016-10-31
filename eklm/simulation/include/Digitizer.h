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

namespace Belle2 {

  namespace EKLM {

    /**
     * @struct DigitizationParams.
     * @brief Digitization parameters.
     *
     * @var DigitizationParams::ADCRange
     * ADC range: 2**(resolution in bits).
     *
     * @var DigitizationParams::digitizationInitialTime
     * Initial digitization time, ns.
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
     */
    struct DigitizationParams {
      double ADCRange;
      double digitizationInitialTime;
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
    };

    /**
     * Set default digitization parameters.
     * @param[out] digPar Digitization parameters.
     */
    void setDefDigitizationParams(struct DigitizationParams* digPar);

  }

}

#endif

