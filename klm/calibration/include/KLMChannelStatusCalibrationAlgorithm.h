/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <calibration/CalibrationAlgorithm.h>
#include <klm/dataobjects/KLMChannelMapValue.h>
#include <klm/dbobjects/KLMChannelStatus.h>

namespace Belle2 {

  /**
   * KLM channel status calibration algorithm.
   */
  class KLMChannelStatusCalibrationAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor.
     */
    KLMChannelStatusCalibrationAlgorithm();

    /**
     * Destructor.
     */
    ~KLMChannelStatusCalibrationAlgorithm();

    /**
     * Calibration.
     */
    CalibrationAlgorithm::EResult calibrate() override;

    /**
     * Set whether the calibration is forced (calibrate even for
     * insufficient average number of hits).
     */
    void setForcedCalibration(bool forcedCalibration)
    {
      m_ForcedCalibration = forcedCalibration;
    }

    /**
     * Set minimal number of hits per module for module-based calibration.
     */
    void setMinimalModuleHitNumber(unsigned int minimalModuleHitNumber)
    {
      m_MinimalModuleHitNumber = minimalModuleHitNumber;
    }

    /**
     * Set maximal absolute value of the logarithm (base 10) of the ratio of
     * the number of hits in this sector and the average number of hits.
     */
    void setMaximalLogSectorHitsRatio(double maximalLogSectorHitsRatio)
    {
      m_MaximalLogSectorHitsRatio = maximalLogSectorHitsRatio;
    }

    /**
     * Get minimal average number of hits per channel required for calibration.
     */
    double getMinimalAverageHitNumber() const
    {
      return m_MinimalAverageHitNumber;
    }

    /**
     * Set minimal average number of hits per channel required for calibration.
     */
    void setMinimalAverageHitNumber(double minimalAverageHitNumber)
    {
      m_MinimalAverageHitNumber = minimalAverageHitNumber;
    }

    /**
     * Get module status (last calibration).
     */
    KLMChannelStatus& getModuleStatus() const
    {
      return *m_ModuleStatus;
    }

    /**
     * Get channel status (last calibration).
     */
    KLMChannelStatus& getChannelStatus() const
    {
      return *m_ChannelStatus;
    }

    /**
     * Get channel hit map.
     */
    const KLMChannelMapValue<unsigned int>& getHitMapChannel() const
    {
      return m_HitMapChannel;
    }

    /**
     * Get module hit map.
     */
    const KLMChannelMapValue<unsigned int>& getHitMapModule() const
    {
      return m_HitMapModule;
    }

    /**
     * Get sector hit map.
     */
    const KLMChannelMapValue<unsigned int>& getHitMapSector() const
    {
      return m_HitMapSector;
    }

    /**
     * Get module active-channel map.
     */
    const KLMChannelMapValue<unsigned int>& getModuleActiveChannelMap() const
    {
      return m_ModuleActiveChannelMap;
    }

    /**
     * Get total hit number.
     */
    unsigned int getTotalHitNumber() const
    {
      return m_TotalHitNumber;
    }

    /**
     * Get number of hits in EKLM.
     */
    unsigned int getHitNumberEKLM() const
    {
      return m_HitNumberEKLM;
    }

    /**
     * Get number of hits in BKLM.
     */
    unsigned int getHitNumberBKLM() const
    {
      return m_HitNumberBKLM;
    }

  protected:

    /**
     * Caliobrate sector.
     *
     * @param[in] sector
     * sector number.
     *
     * @param[in] averageHitsActiveSector
     * Average number of hits per active sector.
     */
    void calibrateSector(uint16_t sector, double averageHitsActiveSector);

    /**
     * Calibrate module.
     * @param[in] module Module number.
     */
    void calibrateModule(uint16_t module);

    /**
     * Calibrate channel.
     * @param[in] channel Channel number.
     */
    void calibrateChannel(uint16_t channel);

    /**
     * Whether the calibration is forced (calibrate even for
     * insufficient average number of hits).
     */
    bool m_ForcedCalibration = false;

    /** Minimal module hit number for module-based calibration. */
    unsigned int m_MinimalModuleHitNumber = 1;

    /** Minimal average number of hits per channel required for calibration. */
    double m_MinimalAverageHitNumber = 25;

    /**
     * Maximal absolute value of the logarithm (base 10) of the ratio of
     * the number of hits in this sector and the average number of hits.
     */
    double m_MaximalLogSectorHitsRatio = 1;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Module status. */
    KLMChannelStatus* m_ModuleStatus = nullptr;

    /** Channel status. */
    KLMChannelStatus* m_ChannelStatus = nullptr;

    /** Channel hit map. */
    KLMChannelMapValue<unsigned int> m_HitMapChannel;

    /** Module hit map. */
    KLMChannelMapValue<unsigned int> m_HitMapModule;

    /** Sector hit map. */
    KLMChannelMapValue<unsigned int> m_HitMapSector;

    /** Module active-channel map (number of active channels in module). */
    KLMChannelMapValue<unsigned int> m_ModuleActiveChannelMap;

    /** Total hit number. */
    unsigned int m_TotalHitNumber = 0;

    /** Number of hits in EKLM. */
    unsigned int m_HitNumberEKLM = 0;

    /** Number of hits in BKLM. */
    unsigned int m_HitNumberBKLM = 0;

  };

}
