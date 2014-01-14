#ifndef TelDQMMODULE_H_
#define TelDQMMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** TEL DQM Module */
  class TelDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of TEL planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 1,2
     */
    enum {
      c_nTELPlanes = 6,
      c_firstTELPlane = 1,
      c_lastTELPlane = 6,
    };

    /** Constructor */
    TelDQMModule();
    /* Destructor */
    virtual ~TelDQMModule();

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
      return c_firstTELPlane + index;
    }
    inline int planeToIndex(int iPlane) const {
      return iPlane - c_firstTELPlane;
    }
    /** This is a shortcut to getting TEL::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const TEL::SensorInfo& getInfo(int index) const;

    std::string m_storeDigitsName;        /**< TELDigits StoreArray name */
    std::string m_storeClustersName;      /**< TELClusters StoreArray name */
    std::string m_relClusterDigitName;    /**< TELClustersToTELDigits RelationArray name */
    std::string m_histogramDirectoryName;  /**< Name of the folder in the histo file */

    // +1 in dimensions to protect against noisy VXDID values.
    TH1F* m_fired[c_nTELPlanes];          /**< Fired in u pixels per event by plane */
    TH1F* m_clusters[c_nTELPlanes];       /**< clusters per event by plane */
    TH1F* m_hitMapU[c_nTELPlanes];        /**< Hitmaps for u-pixels by plane */
    TH1F* m_hitMapV[c_nTELPlanes];        /**< Hitmaps for v-pixels by plane */
    TH1F* m_sizeU[c_nTELPlanes];          /**< u size by plane */
    TH1F* m_sizeV[c_nTELPlanes];          /**< v size by plane */
    TH1F* m_size[c_nTELPlanes];           /**< size by plane */
    TH2F* m_correlationsHitMaps[c_nTELPlanes* c_nTELPlanes];   /**< Correlations and hit maps */
  };

  inline const TEL::SensorInfo& TelDQMModule::getInfo(int index) const
  {
    int iPlane = indexToPlane(index);
    VxdID sensorID;
    if (iPlane <= 3)
      sensorID = VxdID(7 , 2, iPlane);
    else
      sensorID = VxdID(7 , 3, iPlane);
    return dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}
#endif

