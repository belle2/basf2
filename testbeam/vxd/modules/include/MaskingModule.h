#ifndef MaskingMODULE_H_
#define MaskingMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"
#include <sys/time.h>
#include <memory>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <boost/format.hpp>
#include <limits>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <svd/online/SVDIgnoredStripsMap.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>



// Include BASF2 Base stuff
//#include <framework/core/Module.h>
//#include <framework/core/Environment.h>

//#include <framework/datastore/DataStore.h>
//#include <framework/datastore/StoreObjPtr.h>
//#include <framework/dataobjects/EventMetaData.h>

//#include <vxd/dataobjects/VxdID.h>
//#include <rawdata/dataobjects/RawFTSW.h>

namespace Belle2 {

  /** PXD DQM Module */
  class MaskingModule : public HistoModule {  // <- derived from HistoModule class

  public:

//    /** Simple structure for a pixel, u = map_pixel.first, v = map_pixel.second */
//    typedef std::pair<unsigned short, unsigned short> map_pixel;
    /** Number of PXD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 1,2
     */
    enum {
      c_nTBPlanes = 12,
      c_firstTBPlane = 1,
      c_lastTBPlane = 12,
      c_nPXDPlanes = 2,
      c_firstPXDPlane = 1,
      c_lastPXDPlane = 2,
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 3,
      c_lastSVDPlane = 6,
      c_nTelPlanes = 6,
      c_firstTelPlane = 1,
      c_lastTelPlane = 6,
      c_nTelPlanesFrontGrup = 3,
      c_MissingPlaneIndex = 3,
    };

    /** Constructor */
    MaskingModule();
    /* Destructor */
    virtual ~MaskingModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    int CallSVDFilter(const SVDCluster* cluster);
    int CallFullTrackFilter(const StoreArray<PXDCluster>* storePXDClusters,
                            const StoreArray<SVDCluster>* storeSVDClusters,
                            const StoreArray<PXDCluster>* storeTelClusters);
    int CallCorrelation(const StoreArray<PXDCluster>* storePXDClusters,
                        const StoreArray<SVDCluster>* storeSVDClusters,
                        const StoreArray<PXDCluster>* storeTelClusters,
                        const PXDCluster* clusterPXD);
    int CallCorrelation(const StoreArray<PXDCluster>* storePXDClusters,
                        const StoreArray<SVDCluster>* storeSVDClusters,
                        const StoreArray<PXDCluster>* storeTelClusters,
                        const SVDCluster* clusterSVD);
    int CallCorrelationPXDPXD(const PXDCluster* clusterPXD1, const PXDCluster* clusterPXD2, int iPlane1, int iPlane2);
    int CallCorrelationPXDSVD(const PXDCluster* clusterPXD, const SVDCluster* clusterSVD, int iPlane1, int iPlane2);
    int CallCorrelationSVDSVD(const SVDCluster* clusterSVD1, const SVDCluster* clusterSVD2, int iPlane1, int iPlane2);


    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:
    /** Utility functions to convert indices to plane numbers and v.v.,
     * and to protect against range errors.
     */
    inline int indexToPlanePXD(int index) const
    {
      return c_firstPXDPlane + index;
    }
    inline int planeToIndexPXD(int iPlane) const
    {
      return iPlane - c_firstPXDPlane;
    }
    inline int indexToPlaneSVD(int index) const
    {
      return c_firstPXDPlane + index;
    }
    inline int planeToIndexSVD(int iPlane) const
    {
      return iPlane - c_firstPXDPlane;
    }
    inline int indexToPlaneTel(int index) const
    {
      return c_firstTelPlane + index;
    }
    inline int planeToIndexTel(int iPlane) const
    {
      return iPlane - c_firstTelPlane;
    }
    // return global TB plane 1 .. 12:
    inline int indexVXDToTBPlane(int index) const      // from local index
    {
      return c_firstTelPlane + c_nTelPlanesFrontGrup + index;
    }
    inline int indexTelToTBPlane(int index) const      // from local index
    {
      return index > c_nTelPlanesFrontGrup - c_firstTelPlane ?
             c_firstTelPlane + index + c_nPXDPlanes + c_nSVDPlanes :
             c_firstTelPlane + index;
    }
    inline int indexToTBPlane(int index) const         // from global index
    {
      return c_firstTelPlane + index;
    }
    // return global TB index 0 .. 11:
    inline int planeToTBIndex(int iPlane) const        // from global plane
    {
      return iPlane - c_firstTelPlane;
    }
    inline int planeVXDToTBIndex(int iPlane) const     // from local plane
    {
      return iPlane - c_firstTelPlane + c_nTelPlanesFrontGrup;
    }
    inline int planeTelToTBIndex(int iPlane) const     // from local plane
    {
      return iPlane > c_nTelPlanesFrontGrup ?
             iPlane - c_firstTelPlane + c_nPXDPlanes + c_nSVDPlanes :
             iPlane - c_firstTelPlane;
    }

    /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const PXD::SensorInfo& getInfoPXD(int index) const;

    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (2,3,4,5), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const SVD::SensorInfo& getInfoSVD(int index) const;

    /** This is a shortcut to getting TEL::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,9,10,11), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const TEL::SensorInfo& getInfoTel(int index) const;

    std::string m_storePXDDigitsName;        /**< PXDDigits StoreArray name */
    std::string m_storePXDClustersName;      /**< PXDClusters StoreArray name */
    std::string m_relPXDClusterDigitName;    /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_storeSVDDigitsName;        /**< SVDDigits StoreArray name */
    std::string m_storeSVDClustersName;      /**< SVDClusters StoreArray name */
    std::string m_relSVDClusterDigitName;    /**< SVDClustersToPXDDigits RelationArray name */
    std::string m_storeTelDigitsName;        /**< TelDigits StoreArray name */
    std::string m_storeTelClustersName;      /**< TelClusters StoreArray name */
    std::string m_relTelClusterDigitName;    /**< TelClustersToPXDDigits RelationArray name */
    std::string m_storeFramesName;           /**< PXDFrames StoreArray name */
    std::string m_histogramDirectoryName;    /**< Name of the histogram directory in ROOT file */
    std::string m_MaskDirectoryPath;         /**< Path to the mask file directory */
    long m_nEventsProcess;                   /**< Number of events to process */
    long m_nRealEventsProcess;               /**< Real Number of events to process */
    float m_nEventsProcessFraction;          /**< Fraction of events to process to expected No. */
    long m_nEventsPlane[c_nTBPlanes];        /**< How many events in plane */
    int m_AppendMaskFile;                    /**< Set option for append of existing file or recreate new list */
    float m_PXDCutSeedL;                     /**< PXD - lower seed cut for acceptans of cluster */
    float m_PXDCutSeedH;                     /**< PXD - higher seed cut for acceptans of cluster */
    int m_SVDStrongMasking;                  /**< 1: strong SVD masking with time shape; 2: correlation masking */
    int m_StatAllEvents;                     /**< Statistics: all events */
    int m_StatSelEvents;                     /**< Statistics: selected events */
    float m_StatEverageOccupancy[c_nTBPlanes * 2]; /**< Statistics: average occupancy for every plane and direction */

    int m_MaskingStep;                       /**< Set masking step: 0: prepare SVD time shape; 1: apply SVD time shape and create masking */

    // +1 in dimensions to protect against noisy VXDID values.
    TH2F* m_SVDHitMapU[c_nSVDPlanes];        /**< SVD hitmaps for u strips and timestamp by plane */
    TH2F* m_SVDHitMapV[c_nSVDPlanes];        /**< SVD hitmaps for v strips and timestamp by plane */
    TH2F* m_SVDAvrChargeU[c_nSVDPlanes];     /**< SVD average charge for u strips and timestamp by plane */
    TH2F* m_SVDAvrChargeV[c_nSVDPlanes];     /**< SVD average charge for v strips and timestamp by plane */
    TH2F* m_SVDClusterTimeU[c_nSVDPlanes];   /**< SVD cluster time every strip for u by plane */
    TH2F* m_SVDClusterTimeV[c_nSVDPlanes];   /**< SVD cluster time every strip for v by plane */
    TH1F* m_SVDClusterTimeMapU[c_nSVDPlanes];/**< SVD cluster time for u by plane */
    TH1F* m_SVDClusterTimeMapV[c_nSVDPlanes];/**< SVD cluster time for v by plane */

    TH2F* m_CorrNeighboreYu[c_nTBPlanes - 1]; /**< Correlations in Y of space points between neighbore planes */
    TH2F* m_CorrNeighboreZv[c_nTBPlanes - 1]; /**< Correlations in Z of space points between neighbore planes */
    TH2F* m_hitMapUV[c_nTBPlanes];           /**< Hitmaps for pixels by plane */
    TH2F* m_digitMapUV[c_nTBPlanes];         /**< Digitmaps for pixels by plane */
    TH1F* m_digitCharge[c_nTBPlanes];        /**< Digit charge by plane */
    TH1F* m_clusterCharge[c_nTBPlanes];      /**< Cluster charge by plane */
    TH1F* m_seed[c_nTBPlanes];               /**< seed by plane */
  };

  inline const PXD::SensorInfo& MaskingModule::getInfoPXD(int index) const  // TODO for TB 2016 this macro must be revrite correct
  {
    // for only index: 0,1.
    int iPlane = indexToPlanePXD(index);
    // VxdID sensorID(iPlane, 1, iPlane);
    VxdID sensorID(iPlane, 1, 2);
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const SVD::SensorInfo& MaskingModule::getInfoSVD(int index) const  // TODO for TB 2016 this macro must be revrite correct
  {
    // for only index: 2,3,4,5.
    int iPlane = indexToPlaneSVD(index);
    // VxdID sensorID(iPlane, 1, iPlane);
    VxdID sensorID(iPlane, 1, 2);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const TEL::SensorInfo& MaskingModule::getInfoTel(int index) const
  {
    // for only index: 0,1,2,3,4,5.
    int iPlane = indexToPlaneTel(index);
    VxdID sensorID;
    if (iPlane <= 3)
      sensorID = VxdID(7 , 2, iPlane);
    else if (iPlane >= 4)
      sensorID = VxdID(7 , 3, iPlane);
    return dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}
#endif

