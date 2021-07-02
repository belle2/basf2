/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store KLM scintillator simulation parameters in the database.
   */
  class KLMScintillatorDigitizationParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMScintillatorDigitizationParameters();

    /**
     * Destructor.
     */
    ~KLMScintillatorDigitizationParameters();

    /**
     * Get ADC range.
     */
    int getADCRange() const
    {
      return m_ADCRange;
    }

    /**
     * Set ADC range.
     */
    void setADCRange(int range)
    {
      m_ADCRange = range;
    }

    /**
     * Get ADC sampling time in TDC periods.
     */
    int getADCSamplingTDCPeriods() const
    {
      return m_ADCSamplingTDCPeriods;
    }

    /**
     * Set ADC sampling time in TDC periods.
     */
    void setADCSamplingTDCPeriods(int periods)
    {
      m_ADCSamplingTDCPeriods = periods;
    }

    /**
     * Get number of digitizations (points) in one sample.
     */
    int getNDigitizations() const
    {
      return m_NDigitizations;
    }

    /**
     * Set number of digitizations (points) in one sample.
     */
    void setNDigitizations(int digitizations)
    {
      m_NDigitizations = digitizations;
    }

    /**
     * Get ADC pedestal.
     */
    float getADCPedestal() const
    {
      return m_ADCPedestal;
    }

    /**
     * Set ADC pedestal.
     */
    void setADCPedestal(float pedestal)
    {
      m_ADCPedestal = pedestal;
    }

    /**
     * Get ADC photoelectron amplitude.
     */
    float getADCPEAmplitude() const
    {
      return m_ADCPEAmplitude;
    }

    /**
     * Set ADC photoelectron amplitude.
     */
    void setADCPEAmplitude(float amplitude)
    {
      m_ADCPEAmplitude = amplitude;
    }

    /**
     * Get ADC readout corresponding to saturation.
     */
    int getADCThreshold() const
    {
      return m_ADCThreshold;
    }

    /**
     * Set ADC readout corresponding to saturation.
     */
    void setADCThreshold(int threshold)
    {
      m_ADCThreshold = threshold;
    }

    /**
     * Get ADC readout corresponding to saturation.
     */
    int getADCSaturation() const
    {
      return m_ADCSaturation;
    }

    /**
     * Set ADC readout corresponding to saturation.
     */
    void setADCSaturation(int saturation)
    {
      m_ADCSaturation = saturation;
    }

    /**
     * Get number of photoelectrons / 1 MeV.
     */
    float getNPEperMeV() const
    {
      return m_NPEperMeV;
    }

    /**
     * Set number of photoelectrons / 1 MeV.
     */
    void setNPEperMeV(float npe)
    {
      m_NPEperMeV = npe;
    }

    /**
     * Get cosine of maximal light capture angle (by fiber).
     */
    float getMinCosTheta() const
    {
      return m_MinCosTheta;
    }

    /**
     * Set cosine of maximal light capture angle (by fiber).
     */
    void setMinCosTheta(float minCosTheta)
    {
      m_MinCosTheta = minCosTheta;
    }

    /**
     * Get mirror reflective index.
     */
    float getMirrorReflectiveIndex() const
    {
      return m_MirrorReflectiveIndex;
    }

    /**
     * Set mirror reflective index.
     */
    void setMirrorReflectiveIndex(float reflectiveIndex)
    {
      m_MirrorReflectiveIndex = reflectiveIndex;
    }

    /**
     * Get scintillator deexcitation time.
     */
    float getScintillatorDeExcitationTime() const
    {
      return m_ScintillatorDeExcitationTime;
    }

    /**
     * Set scintillator deexcitation time.
     */
    void setScintillatorDeExcitationTime(float time)
    {
      m_ScintillatorDeExcitationTime = time;
    }

    /**
     * Get fiber deexcitation time.
     */
    float getFiberDeExcitationTime() const
    {
      return m_FiberDeExcitationTime;
    }

    /**
     * Set fiber deexcitation time.
     */
    void setFiberDeExcitationTime(float time)
    {
      m_FiberDeExcitationTime = time;
    }

    /**
     * Get speed of light in fiber.
     */
    float getFiberLightSpeed() const
    {
      return m_FiberLightSpeed;
    }

    /**
     * Set speed of light in fiber.
     */
    void setFiberLightSpeed(float lightSpeed)
    {
      m_FiberLightSpeed = lightSpeed;
    }

    /**
     * Get attenuation length in fiber.
     */
    float getAttenuationLength() const
    {
      return m_AttenuationLength;
    }

    /**
     * Set attenuation length in fiber.
     */
    void setAttenuationLength(float length)
    {
      m_AttenuationLength = length;
    }

    /**
     * Get attenuation frequency of a single photoelectron pulse.
     */
    float getPEAttenuationFrequency() const
    {
      return m_PEAttenuationFrequency;
    }

    /**
     * Set attenuation frequency of a single photoelectron pulse.
     */
    void setPEAttenuationFrequency(float frequency)
    {
      m_PEAttenuationFrequency = frequency;
    }

    /**
     * Get mean for SiPM backgrounds.
     */
    float getMeanSiPMNoise() const
    {
      return m_MeanSiPMNoise;
    }

    /**
     * Set mean for SiPM backgrounds.
     */
    void setMeanSiPMNoise(int noise)
    {
      m_MeanSiPMNoise = noise;
    }

    /**
     * Check if background is enabled in fitting.
     */
    bool getEnableConstBkg() const
    {
      return m_EnableConstBkg;
    }

    /**
     * Set if background is enabled in fitting.
     */
    void setEnableConstBkg(bool enable)
    {
      m_EnableConstBkg = enable;
    }

  private:

    /** ADC range: 2**(resolution in bits). */
    int m_ADCRange;

    /** ADC sampling time (TDC periods). */
    int m_ADCSamplingTDCPeriods;

    /** Number of digitizations (points) in one sample. */
    int m_NDigitizations;

    /** ADC pedestal. */
    float m_ADCPedestal;

    /** ADC photoelectron amplitude. */
    float m_ADCPEAmplitude;

    /** ADC threshold. */
    float m_ADCThreshold;

    /** ADC readout corresponding to saturation. */
    int m_ADCSaturation;

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

    /** Class version. */
    ClassDef(Belle2::KLMScintillatorDigitizationParameters, 2);

  };

}
