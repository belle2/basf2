/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMPlaneArrayIndex.h>
#include <klm/dbobjects/KLMStripEfficiency.h>

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {

  /**
   * KLM channel status calibration algorithm.
   */
  class KLMStripEfficiencyAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Calibration stage.
     */
    enum CalibrationStage {

      /** Check of set of planes with determined efficiency. */
      c_MeasurablePlaneCheck,

      /** Measurement. */
      c_EfficiencyMeasurement,

    };

    /**
     * Constructor.
     */
    KLMStripEfficiencyAlgorithm();

    /**
     * Copy constructor (disabled).
     */
    KLMStripEfficiencyAlgorithm(const KLMStripEfficiencyAlgorithm&) = delete;

    /**
     * Operator = (disabled).
     */
    KLMStripEfficiencyAlgorithm& operator=(const KLMStripEfficiencyAlgorithm&) = delete;

    /**
     * Destructor.
     */
    ~KLMStripEfficiencyAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

    /**
     * Set output file name.
     */
    void setOutputFileName(const char* outputFileName)
    {
      m_OutputFileName = outputFileName;
    }

    /**
     * Set whether the calibration is forced.
     */
    void setForcedCalibration(bool forcedCalibration)
    {
      m_ForcedCalibration = forcedCalibration;
    }

    /**
     * Set calibration stage.
     */
    void setCalibrationStage(enum CalibrationStage calibrationStage)
    {
      m_CalibrationStage = calibrationStage;
    }

    /**
     * Set minimal number of ExtHits.
     */
    void setMinimalExtHits(int minimalExtHits)
    {
      m_MinimalExtHits = minimalExtHits;
    }

    /**
     * Set requested precision of efficiency measurement.
     */
    void setRequestedPrecision(float requestedPrecision)
    {
      m_RequestedPrecision = requestedPrecision;
    }

    /**
     * Get requested precision of efficiency measurement.
     */
    float getRequestedPrecision() const
    {
      return m_RequestedPrecision;
    }

    /**
     * Get achieved precision of efficiency measurement (the worst one).
     */
    float getAchievedPrecision() const
    {
      return m_AchievedPrecision;
    }

    /**
     * Get number of matched digits.
     */
    int getMatchedDigits() const
    {
      return m_MatchedDigits;
    }

    /**
     * Get efficiency.
     */
    float* getEfficiency() const
    {
      return m_Efficiency;
    }

    /**
     * Get total number of ExtHits.
     */
    int getExtHits() const
    {
      return m_ExtHits;
    }

    /**
     * Get number of ExtHits per plane.
     */
    int* getExtHitsPlane() const
    {
      return m_ExtHitsPlane;
    }

    /**
     * Get number of new measured planes.
     * @param[in] efficiency Other efficiency measurement.
     */
    int newMeasuredPlanes(float* efficiency) const;

    /**
     * Get number of new planes with ExhHits.
     * @param[in] efficiency Number of ExtHits for other measurement.
     */
    int newExtHitsPlanes(int* extHitsPlane) const;

  private:

    /** Output root file */
    std::string m_OutputFileName = "TrackMatchedResult.root";

    /** Whether the calibration is forced. */
    bool m_ForcedCalibration = false;

    /** Calibration stage. */
    enum CalibrationStage m_CalibrationStage = c_MeasurablePlaneCheck;

    /** Minimal number of ExtHits per plane. */
    int m_MinimalExtHits = 10;

    /** Requested precision of efficiency measurement. */
    float m_RequestedPrecision = 0.02;

    /** Achieved precision of efficiency measurement. */
    float m_AchievedPrecision = 0;

    /** Number of matched digits. */
    int m_MatchedDigits = 0;

    /** Efiiciency. */
    float* m_Efficiency;

    /** Number of ExtHits. */
    int m_ExtHits;

    /** Number of ExtHits per plane. */
    int* m_ExtHitsPlane;

    /** Element numbers */
    const KLMElementNumbers* m_ElementNumbers;

    /** Plane array index. */
    const KLMPlaneArrayIndex* m_PlaneArrayIndex;

    /** Efficiency data object */
    KLMStripEfficiency* m_StripEfficiency;

  };

}
