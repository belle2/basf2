/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMCHANNELDATAIMPORTER_H
#define EKLMCHANNELDATAIMPORTER_H

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
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
     * @param[in] endcap      Endcap number.
     * @param[in] layer       Layer number.
     * @param[in] sector      Sector number.
     * @param[in] plane       Plane number.
     * @param[in] strip       Strip number.
     * @param[in] channelData Channel data.
     */
    void setChannelData(int endcap, int layer, int sector, int plane,
                        int strip, EKLMChannelData* channelData);

    /**
     * Load channel data from calibration data file.
     * @param[in] calibrationData Clibration data ROOT file name.
     * @param[in] thresholdShift  Threshold shift.
     */
    void loadChannelDataCalibration(const char* calibrationData,
                                    int thresholdShift);

    /**
     * Import channel data.
     */
    void importChannelData();

  private:

    /** Channel data. */
    DBImportObjPtr<EKLMChannels> m_Channels;

    /** Low experiment. */
    int m_ExperimentLow;

    /** Low run. */
    int m_RunLow;

    /** High experiment. */
    int m_ExperimentHigh;

    /** High run. */
    int m_RunHigh;

  };

}

#endif

