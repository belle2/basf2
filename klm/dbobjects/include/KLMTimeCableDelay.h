/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 Headers. */

namespace Belle2 {

  /**
   * Class to store BKLM delay time coused by cable in the database.
   */
  class KLMTimeCableDelay : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMTimeCableDelay();

    /**
     * Destructor.
     */
    ~KLMTimeCableDelay();

    /**
     * Set time calibration data.
     * @param[in] strip Strip number.
     * @param[in] dat TimeCalibration data.
     */
    void setTimeShift(uint16_t, double);

    /**
     * Get time calibration data.
     * @param[in] strip Strip number.
     */
    double getTimeShift(uint16_t) const;

    /**
     * Clean time calibration data.
     */
    void cleanTimeShift();

    /**
     * Get effective light speed for scintillator.
     */
    double getEffLightSpeed() const;
    double getEffLightSpeedRPC() const;

    /**
     * Set effective light speed.
     * @param[in] lightSpeed Effective light speed.
     */
    void setEffLightSpeed(double lightSpeed);
    void setEffLightSpeedRPC(double lightSpeed);

    /**
     * Get amplitude dependence time constant.
     */
    double getAmpTimeConstant() const;
    double getAmpTimeConstantRPC() const;

    /**
     * Set amplitude dependence time constant.
     * @param[in] amplitudeTimeConstant Amplitude dependence time constant.
     */
    void setAmpTimeConstant(double amplitudeTimeConstant);
    void setAmpTimeConstantRPC(double amplitudeTimeConstant);


  private:

    /** Time calibration data. */
    std::map<uint16_t, double> m_timeShift;

    /** Effective light speed. */
    double m_effLightSpeed;
    double m_effLightSpeedRPC;

    /** Amplitude dependence time constant. */
    double m_ampTimeConstant;
    double m_ampTimeConstantRPC;

    /** Class version. */
    ClassDef(KLMTimeCableDelay, 1);

  };

}
