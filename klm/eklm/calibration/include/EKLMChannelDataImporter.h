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

/* KLM headers. */
#include <klm/dbobjects/eklm/EKLMChannels.h>

/* Belle 2 headers. */
#include <framework/database/DBImportObjPtr.h>

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class EKLMChannelDataImporter {

  public:

    /**
     * Constructor.
     */
    EKLMChannelDataImporter();

    /**
     * Destructor.
     */
    ~EKLMChannelDataImporter();

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Load specific channel data to all channels.
     * @param[in] channelData Channel data.
     */
    void loadChannelData(EKLMChannelData* channelData);

    /**
     * Set channel data.
     * @param[in] section     Section number.
     * @param[in] layer       Layer number.
     * @param[in] sector      Sector number.
     * @param[in] plane       Plane number.
     * @param[in] strip       Strip number.
     * @param[in] channelData Channel data.
     */
    void setChannelData(int section, int layer, int sector, int plane,
                        int strip, EKLMChannelData* channelData);

    /**
     * Load active channels from ROOT file.
     * @param[in] activeChannelsData ROOT file with active channels data.
     */
    void loadActiveChannels(const char* activeChannelsData);

    /**
     * Load high voltage from ROOT file.
     * @param[in] highVoltageData ROOT file with high voltage data.
     */
    void loadHighVoltage(const char* highVoltageData);

    /**
     * Load lookback window from ROOT file.
     * @param[in] lookbackWindowData ROOT file with lookback window data.
     */
    void loadLookbackWindow(const char* lookbackWindowData);

    /**
     * Load thresholds and adjustment voltages from ROOT file.
     * @param[in] thresholdsData ROOT file with threshold and adjustment
     *                           voltage data.
     */
    void loadThresholds(const char* thresholdsData);

    /**
     * Import channel data.
     */
    void importChannelData();

  private:

    /** Channel data. */
    DBImportObjPtr<EKLMChannels> m_Channels;

    /** Low experiment. */
    int m_ExperimentLow = 0;

    /** Low run. */
    int m_RunLow = 0;

    /** High experiment. */
    int m_ExperimentHigh = -1;

    /** High run. */
    int m_RunHigh = -1;

  };

}
