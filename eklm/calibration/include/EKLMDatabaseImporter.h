/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDATABASEIMPORTER_H
#define EKLMDATABASEIMPORTER_H

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMChannels.h>
#include <framework/database/DBImportObjPtr.h>

namespace Belle2 {

  /**
   * EKLM time calibration algorithm.
   */
  class EKLMDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    EKLMDatabaseImporter();

    /**
     * Destructor.
     */
    ~EKLMDatabaseImporter();

    /**
     * Import digitization parameters.
     */
    void importDigitizationParameters();

    /**
     * Import reconstruction parameters.
     */
    void importReconstructionParameters();

    /**
     * Import simulation parameters.
     */
    void importSimulationParameters();

    /**
     * Load default channel data.
     */
    void loadDefaultChannelData();

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
     * Import channel data.
     */
    void importChannelData();

  private:

    /** Channel data. */
    DBImportObjPtr<EKLMChannels> m_Channels;

  };

}

#endif

