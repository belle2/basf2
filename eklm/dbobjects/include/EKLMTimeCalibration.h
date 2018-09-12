/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTIMECALIBRATION_H
#define EKLMTIMECALIBRATION_H

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 Headers. */
#include <eklm/dbobjects/EKLMTimeCalibrationData.h>

namespace Belle2 {

  /**
   * Class to store EKLM time calibration data in the database.
   */
  class EKLMTimeCalibration : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMTimeCalibration();

    /**
     * Destructor.
     */
    ~EKLMTimeCalibration();

    /**
     * Set time calibration data.
     * @param[in] strip Strip number.
     * @param[in] dat     TimeCalibration data.
     */
    void setTimeCalibrationData(uint16_t strip, EKLMTimeCalibrationData* dat);

    /**
     * Get time calibration data.
     * @param[in] strip Strip number.
     */
    const EKLMTimeCalibrationData* getTimeCalibrationData(uint16_t strip) const;

    /**
     * Clean time calibration data.
     */
    void cleanTimeCalibrationData();

    /**
     * Get effective light speed.
     */
    float getEffectiveLightSpeed() const;

    /**
     * Set effective light speed.
     * @param[in] lightSpeed Effective light speed.
     */
    void setEffectiveLightSpeed(float lightSpeed);

    /**
     * Get amplitude dependence time constant.
     */
    float getAmplitudeTimeConstant() const;

    /**
     * Set amplitude dependence time constant.
     * @param[in] amplitudeTimeConstant Amplitude dependence time constant.
     */
    void setAmplitudeTimeConstant(float amplitudeTimeConstant);


  private:

    /** Time calibration data. */
    std::map<uint16_t, EKLMTimeCalibrationData> m_data;

    /** Effective light speed. */
    float m_EffectiveLightSpeed;

    /** Amplitude dependence time constant. */
    float m_AmplitudeTimeConstant;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMTimeCalibration, 2);

  };

}

#endif

