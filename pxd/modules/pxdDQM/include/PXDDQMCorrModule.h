/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Corr Module */
  class PXDDQMCorrModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of PXD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 1,2
     */
    enum {
      c_nPXDPlanes = 2,
      c_firstPXDPlane = 1,
      c_lastPXDPlane = 2,
    };

    /** Constructor */
    PXDDQMCorrModule();
    /* Destructor */
    virtual ~PXDDQMCorrModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:
    /** Utility function to convert index to plane number
     * and to protect against range errors.
     */
    inline int indexToPlane(int index) const
    {
      return c_firstPXDPlane + index;
    }
    /** Utility function to convert plane number to index
     * and to protect against range errors.
     */
    inline int planeToIndex(int iPlane) const
    {
      return iPlane - c_firstPXDPlane;
    }
    /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const PXD::SensorInfo& getInfo(int index) const;

    /** PXDClusters StoreArray name */
    std::string m_storeClustersName;
    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** Storearray for clusters   */
    StoreArray<PXDCluster> m_storeClusters;

    // +1 in dimensions to protect against noisy VXDID values.
    /** Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationU;
    /** Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationV;
    /** Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaU;
    /** Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaV;
  };

  /** Utility function to find sonsor ID
    * @param index Index of the sensor (0,1), _not_ layer number!
    * @return SensorInfo object for the desired sensor.
    */
  inline const PXD::SensorInfo& PXDDQMCorrModule::getInfo(int index) const
  {
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane, 1, iPlane);
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}

