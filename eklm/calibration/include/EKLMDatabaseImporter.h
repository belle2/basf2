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
#include <eklm/dbobjects/EKLMAlignment.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
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
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

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

    /**
     * Load default displacement data (zeros).
     */
    void loadDefaultDisplacement();

    /**
     * Set sector displacement.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in[ dx     dx.
     * @param[in] dy     dy.
     * @param[in] dalpha dalpha.
     */
    void setSectorDisplacement(int endcap, int layer, int sector,
                               float dx, float dy, float dalpha);

    /**
     * Set segment displacement.
     * @param[in] endcap  Endcap number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     * @param[in] dx      dx.
     * @param[in] dy      dy.
     * @param[in] dalpha  dalpha.
     */
    void setSegmentDisplacement(int endcap, int layer, int sector, int plane,
                                int segment, float dx, float dy, float dalpha);

    /**
     * Import displacement data.
     */
    void importDisplacement();

    /**
     * Load default electronics map. This function actually only initializes
     * the database object pointer, but does not load anything. All data should
     * be loaded by calling addSectorLane().
     */
    void loadDefaultElectronicsMap();

    /**
     * Add sector-lane pair.
     * @param[in] endcap           Endcap number.
     * @param[in] layer            Layer number.
     * @param[in] sector           Sector number.
     * @param[in] copper           Copper identifier.
     * @param[in] dataConcentrator Data concentrator number.
     * @param[in] lane             Lane number.
     */
    void addSectorLane(int endcap, int layer, int sector,
                       int copper, int dataConcentrator, int lane);

    /**
     * Import electronics map.
     */
    void importElectronicsMap();

  private:

    /** Channel data. */
    DBImportObjPtr<EKLMChannels> m_Channels;

    /** Displacement. */
    DBImportObjPtr<EKLMAlignment> m_Displacement;

    /** Electronics map. */
    DBImportObjPtr<EKLMElectronicsMap> m_ElectronicsMap;

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

