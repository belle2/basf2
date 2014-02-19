#ifndef PXDDQMMODULE_H_
#define PXDDQMMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMModule : public HistoModule {  // <- derived from HistoModule class

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
    PXDDQMModule();
    /* Destructor */
    virtual ~PXDDQMModule();

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
    /** Utility functions to convert indices to plane numbers and v.v.,
     * and to protect against range errors.
     */
    inline int indexToPlane(int index) const {
      return c_firstPXDPlane + index;
    }
    inline int planeToIndex(int iPlane) const {
      return iPlane - c_firstPXDPlane;
    }
    /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const PXD::SensorInfo& getInfo(int index) const;

    std::string m_storeDigitsName;        /**< PXDDigits StoreArray name */
    std::string m_storeClustersName;      /**< PXDClusters StoreArray name */
    std::string m_relClusterDigitName;    /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_storeFramesName;        /**< PXDFrames StoreArray name */
    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
    float m_PXDCutSeedL;                  /**< PXD - lower seed cut for acceptans of cluster */
    float m_PXDCutSeedH;                  /**< PXD - Higher seed cut for acceptans of cluster */

    // +1 in dimensions to protect against noisy VXDID values.
    TH1F* m_fired[c_nPXDPlanes];          /**< Fired in u pixels per event by plane */
    TH1F* m_clusters[c_nPXDPlanes];       /**< clusters per event by plane */
    TH1F* m_hitMapU[c_nPXDPlanes];        /**< Hitmaps for u-pixels by plane */
    TH1F* m_hitMapV[c_nPXDPlanes];        /**< Hitmaps for v-pixels by plane */
    TH2F* m_hitMapUV[c_nPXDPlanes];       /**< Hitmaps for pixels by plane */
    TH1F* m_digitCharge[c_nPXDPlanes];    /**< Digit charge by plane */
    TH1F* m_clusterCharge[c_nPXDPlanes];  /**< Cluster charge by plane */
    TH1F* m_seed[c_nPXDPlanes];           /**< seed by plane */
    TH1F* m_sizeU[c_nPXDPlanes];          /**< u size by plane */
    TH1F* m_sizeV[c_nPXDPlanes];          /**< v size by plane */
    TH1F* m_size[c_nPXDPlanes];           /**< size by plane */
    TH1F* m_startRow[c_nPXDPlanes];       /**< start row by plane */
    TH1F* m_chargeByStartRow[c_nPXDPlanes]; /** average seed charge by distance from start row */
    TH1F* m_chargeByStartRowCount[c_nPXDPlanes]; /** seed charge count by distance from start row */
    TH1F* m_averageSeedByU[c_nPXDPlanes]; /** average seed charge by u */
    TH1F* m_seedCountsByU[c_nPXDPlanes];  /** seed charge count by u */
    TH1F* m_averageSeedByV[c_nPXDPlanes]; /** average seed charge by v */
    TH1F* m_seedCountsByV[c_nPXDPlanes];  /** seed charge count by v */
  };

  inline const PXD::SensorInfo& PXDDQMModule::getInfo(int index) const
  {
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane, 1, iPlane);
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}
#endif

