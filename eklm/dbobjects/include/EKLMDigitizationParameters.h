/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZATIONPARAMETERS_H
#define EKLMDIGITIZATIONPARAMETERS_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store EKLM simulation in the database.
   */
  class EKLMDigitizationParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMDigitizationParameters();

    /**
     * Destructor.
     */
    ~EKLMDigitizationParameters();

    /**
     * Get ADC range.
     */
    float getADCRange() const;

    /**
     * Set ADC range.
     */
    void setADCRange(float range);

    /**
     * Get initial digitization time.
     */
    float getDigitizationInitialTime() const;

    /**
     * Set initial digitization time.
     */
    void setDigitizationInitialTime(float time);

    /**
     * Get ADC sampling time.
     */
    float getADCSamplingTime() const;

    /**
     * Set ADC sampling time.
     */
    void setADCSamplingTime(float time);

    /**
     * Get number of digitizations (points) in one sample.
     */
    int getNDigitizations() const;

    /**
     * Set number of digitizations (points) in one sample.
     */
    void setNDigitizations(int digitizations);

    /**
     * Get ADC pedestal.
     */
    float getADCPedestal() const;

    /**
     * Set ADC pedestal.
     */
    void setADCPedestal(float pedestal);

    /**
     * Get ADC photoelectron amplitude.
     */
    float getADCPEAmplitude() const;

    /**
     * Set ADC photoelectron amplitude.
     */
    void setADCPEAmplitude(float amplitude);

    /**
     * Get ADC readout corresponding to saturation.
     */
    float getADCSaturation() const;

    /**
     * Set ADC readout corresponding to saturation.
     */
    void setADCSaturation(float saturation);

    /**
     * Get number of photoelectrons / 1 MeV.
     */
    float getNPEperMeV() const;

    /**
     * Set number of photoelectrons / 1 MeV.
     */
    void setNPEperMeV(float npe);

    /**
     * Get cosine of maximal light capture angle (by fiber).
     */
    float getMinCosTheta() const;

    /**
     * Set cosine of maximal light capture angle (by fiber).
     */
    void setMinCosTheta(float minCosTheta);

    /**
     * Get mirror reflective index.
     */
    float getMirrorReflectiveIndex() const;

    /**
     * Set mirror reflective index.
     */
    void setMirrorReflectiveIndex(float reflectiveIndex);

    /**
     * Get scintillator deexcitation time.
     */
    float getScintillatorDeExcitationTime() const;

    /**
     * Set scintillator deexcitation time.
     */
    void setScintillatorDeExcitationTime(float time);

    /**
     * Get fiber deexcitation time.
     */
    float getFiberDeExcitationTime() const;

    /**
     * Set fiber deexcitation time.
     */
    void setFiberDeExcitationTime(float time);

    /**
     * Get speed of light in fiber.
     */
    float getFiberLightSpeed() const;

    /**
     * Set speed of light in fiber.
     */
    void setFiberLightSpeed(float lightSpeed);

    /**
     * Get attenuation length in fiber.
     */
    float getAttenuationLength() const;

    /**
     * Set attenuation length in fiber.
     */
    void setAttenuationLength(float length);

    /**
     * Get attenuation frequency of a single photoelectron pulse.
     */
    float getPEAttenuationFrequency() const;

    /**
     * Set attenuation frequency of a single photoelectron pulse.
     */
    void setPEAttenuationFrequency(float frequency);

    /**
     * Get mean for SiPM backgrounds.
     */
    float getMeanSiPMNoise() const;

    /**
     * Set mean for SiPM backgrounds.
     */
    void setMeanSiPMNoise(int noise);

    /**
     * Check if background is enabled in fitting.
     */
    bool getEnableConstBkg() const;

    /**
     * Set if background is enabled in fitting.
     */
    void setEnableConstBkg(bool enable);

    /**
     * Get time resolution (of reconstructed time, not ADC).
     */
    float getTimeResolution() const;

    /**
     * Set time resolution (of reconstructed time, not ADC).
     */
    void setTimeResolution(float resolution);

  private:

    /** ADC range: 2**(resolution in bits). */
    float m_ADCRange;

    /** Initial digitization time, ns. */
    float m_DigitizationInitialTime;

    /** ADC sampling time, ns. */
    float m_ADCSamplingTime;

    /** Number of digitizations (points) in one sample. */
    int m_NDigitizations;

    /** ADC pedestal. */
    float m_ADCPedestal;

    /** ADC photoelectron amplitude. */
    float m_ADCPEAmplitude;

    /** ADC readout corresponding to saturation. */
    float m_ADCSaturation;

    /** Number of photoelectrons / 1 MeV. */
    float m_NPEperMeV;

    /** Cosine of maximal light capture angle (by fiber). */
    float m_MinCosTheta;

    /** Mirror reflective index. */
    float m_MirrorReflectiveIndex;

    /** Scintillator deexcitation time, ns. */
    float m_ScintillatorDeExcitationTime;

    /** Fiber deexcitation time, ns. */
    float m_FiberDeExcitationTime;

    /** Speed of light in fiber, cm/ns. */
    float m_FiberLightSpeed;

    /** Attenuation length in fiber, cm. */
    float m_AttenuationLength;

    /** Attenuation frequency of a single photoelectron pulse, ns^-1. */
    float m_PEAttenuationFrequency;

    /**
     * Mean for SiPM backgrounds. If zero or negative no backgrounds are added.
     */
    float m_MeanSiPMNoise;

    /** Enable background in fitting. */
    bool m_EnableConstBkg;

    /** Time resolution (of reconstructed time, not ADC). */
    float m_TimeResolution;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMDigitizationParameters, 1);

  };

}

#endif

