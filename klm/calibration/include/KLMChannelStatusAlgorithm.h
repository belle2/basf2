/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMChannelMapValue.h>
#include <klm/dbobjects/KLMChannelStatus.h>

/* Belle 2 headers. */
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {

  /**
   * KLM channel status calibration algorithm.
   */
  class KLMChannelStatusAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Calibration results.
     */
    class Results {

      friend class KLMChannelStatusAlgorithm;

    public:

      /**
       * Constructor.
       */
      Results();

      /**
       * Copy constructor.
       */
      Results(const Results& results);

      /**
       * Destructor.
       */
      ~Results();

      /**
       * Get channel status (last calibration).
       */
      KLMChannelStatus& getChannelStatus() const
      {
        return *m_ChannelStatus;
      }

      /**
       * Get module status (last calibration).
       */
      KLMChannelStatus& getModuleStatus() const
      {
        return *m_ModuleStatus;
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
       * Get module hit map (no hot channels).
       */
      const KLMChannelMapValue<unsigned int>& getHitMapModuleNoHot() const
      {
        return m_HitMapModuleNoHot;
      }

      /**
       * Get sector hit map (no hot channels).
       */
      const KLMChannelMapValue<unsigned int>& getHitMapSectorNoHot() const
      {
        return m_HitMapSectorNoHot;
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

    private:

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

      /** Module hit map (no hit channels). */
      KLMChannelMapValue<unsigned int> m_HitMapModuleNoHot;

      /** Sector hit map (no hot channels). */
      KLMChannelMapValue<unsigned int> m_HitMapSectorNoHot;

      /** Module active-channel map (number of active channels in module). */
      KLMChannelMapValue<unsigned int> m_ModuleActiveChannelMap;

      /** Total hit number. */
      unsigned int m_TotalHitNumber = 0;

      /** Number of hits in BKLM. */
      unsigned int m_HitNumberBKLM = 0;

      /** Number of hits in EKLM. */
      unsigned int m_HitNumberEKLM = 0;

      /** Number of hits in BKLM (no hot channels). */
      unsigned int m_HitNumberBKLMNoHot = 0;

      /** Number of hits in EKLM (no hot channels). */
      unsigned int m_HitNumberEKLMNoHot = 0;

    };

    /**
     * Constructor.
     */
    KLMChannelStatusAlgorithm();

    /**
     * Destructor.
     */
    ~KLMChannelStatusAlgorithm();

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
     * Set minimal number of hits in a single channel to mark it as hot.
     */
    void setMinimalHitNumberSingleHotChannel(
      unsigned int minimalHitNumberSingleHotChannel)
    {
      m_MinimalHitNumberSingleHotChannel = minimalHitNumberSingleHotChannel;
    }

    /**
     * Set inimal number of hits to mark to mark the channel as hot.
     */
    void setMinimalHitNumberHotChannel(unsigned int minimalHitNumberHotChannel)
    {
      m_MinimalHitNumberHotChannel = minimalHitNumberHotChannel;
    }

    /**
     * Minimal ratio of number of hits in this channel and average over other
     * channels in this module to mark it as hot.
     */
    void setMinimalHitNumberRatioHotChannel(
      double minimalHitNumberRatioHotChannel)
    {
      m_MinimalHitNumberRatioHotChannel = minimalHitNumberRatioHotChannel;
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
     * Get results.
     */
    const Results* getResults() const
    {
      return &m_Results;
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
    void calibrateSector(KLMSectorNumber sector,
                         double averageHitsActiveSector);

    /**
     * Calibrate module.
     * @param[in] module Module number.
     */
    void calibrateModule(KLMModuleNumber module);

    /**
     * Mark hot channel.
     * @param[in] channel        Channel number.
     * @param[in] moduleHits     Number of hits in this module.
     * @param[in] activeChannels Number of active channels in this module.
     * @return True if the channel is marked as hot.
     */
    bool markHotChannel(KLMChannelNumber channel, unsigned int moduleHits,
                        int activeChannels);

    /**
     * Calibrate channel.
     * @param[in] channel Channel number.
     */
    void calibrateChannel(KLMChannelNumber channel);

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

    /** Minimal number of hits in a single channel to mark it as hot. */
    unsigned int m_MinimalHitNumberSingleHotChannel = 1000;

    /** Minimal number of hits to mark to mark the channel as hot. */
    unsigned int m_MinimalHitNumberHotChannel = 100;

    /**
     * Minimal ratio of number of hits in this channel and average over other
     * channels in this module to mark it as hot.
     */
    double m_MinimalHitNumberRatioHotChannel = 50;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Calibration results. */
    Results m_Results;

  };

}
