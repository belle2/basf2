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
    EKLMTimeCalibrationData* getTimeCalibrationData(uint16_t strip);

    /**
     * Clean time calibration data.
     */
    void cleanTimeCalibrationData();

    /**
     * Set effective light speed.
     * @pparam[in] lightSpeed Effective light speed.
     */
    void setEffectiveLightSpeed(float lightSpeed);

    /**
     * Get effective light speed.
     */
    float getEffectiveLightSpeed() const;

  private:

    /** Time calibration data. */
    std::map<uint16_t, EKLMTimeCalibrationData> m_data;

    /** Effective light speed. */
    float m_EffectiveLightSpeed;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMTimeCalibration, 1);

  };

}

#endif

