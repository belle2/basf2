#ifndef TelxVXDMODULE_H_
#define TelxVXDMODULE_H_

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

namespace Belle2 {

  /** TB DQM Module */
  class TelxVXDModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of TB planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 3,4,5,6
     */
    enum {
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 6,
      c_lastSVDPlane = 9,
      c_nPXDPlanes = 2,
      c_firstPXDPlane = 4,
      c_lastPXDPlane = 5,
      c_nVXDPlanes = 6,
      c_firstVXDPlane = 4,
      c_lastVXDPlane = 9,
      c_nTBPlanes = 12,
      c_firstTBPlane = 1,
      c_lastTBPlane = 12,
      c_nTelAPlanes = 3,
      c_firstTelAPlane = 1,
      c_lastTelAPlane = 3,
      c_nTelBPlanes = 3,
      c_firstTelBPlane = 10,
      c_lastTelBPlane = 12,
    };

    /** Constructor */
    TelxVXDModule();
    /* Destructor */
    virtual ~TelxVXDModule();

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
      return c_firstTBPlane + index;
    }
    inline int planeToIndex(int iPlane) const {
      return iPlane - c_firstTBPlane;
    }
    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (5,6,7,8), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const SVD::SensorInfo& getInfoSVD(int index) const;
    /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (3,4), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const PXD::SensorInfo& getInfoPXD(int index) const;
    /** This is a shortcut to getting TEL::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,9,10,11), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const TEL::SensorInfo& getInfoTel(int index) const;

    std::string m_storeSVDDigitsName;     /**< SVDDigits StoreArray name */
    std::string m_storeTelDigitsName;     /**< TelDigits StoreArray name */
    std::string m_storePXDClustersName;   /**< PXDClusters StoreArray name */
    std::string m_storeTelClustersName;   /**< TelClusters StoreArray name */
    std::string m_storeSVDClustersName;   /**< SVDClusters StoreArray name */
    std::string m_relSVDClusterDigitName; /**< SVDClustersToSVDDigits RelationArray name */
    std::string m_relTelClusterDigitName; /**< TelClustersToTelDigits RelationArray name */
    std::string m_histogramDirectoryName; /**< Name of the ROOT file directory for these histograms */

    int m_SwapAxis;     /**< 0: u<->u, 1: u<->v correlations */
    int m_UseSP;        /**< 1: use space points for hitmaps and correlations */
    std::vector<int> m_ModulesForCheck; /**< List of modules for check in order as is below: */
    int m_DUTPXD;       /**< PXD for correlations: 3,4 */
    int m_DUTSVDFwd;    /**< forward SVD for correlations: 5..8 */
    int m_DUTSVDBwd;    /**< backward SVD for correlations: 5..8 */
    int m_DUTTelFwd;    /**< forward telescope for correlations: 0..2 */
    int m_DUTTelBwd;    /**< backward telescope for correlations: 9..11 */

    // +1 in dimensions to protect against noisy VXDID values.
    TH1F* m_chargePXD2;                 /**< charge by plane */
    TH1F* m_chargeUSVD1;                /**< u charge by plane */
    TH1F* m_chargeVSVD1;                /**< v charge by plane */
    TH1F* m_chargeTel3;                 /**< charge by plane */
    TH1F* m_chargeTel4;                 /**< charge by plane */
    TH2F* m_hitMapTel3;                 /**< Hit maps, local uv coordinates*/
    TH2F* m_hitMapTel4;                 /**< Hit maps, local uv coordinates*/
    TH2F* m_hitMapPXD2;                 /**< Hit maps, local uv coordinates*/
    TH2F* m_hitMapSVD1;                 /**< Hit maps, local uv coordinates*/
    TH2F* m_hitMapSVD4;                 /**< Hit maps, local uv coordinates*/
    TH2F* m_correlationsTel3SVD1[2];    /**< Correlations, local uv coordinates*/
    TH2F* m_correlationsTel4SVD4[2];    /**< Correlations, local uv coordinates*/
    TH2F* m_correlationsTel3PXD2[2];    /**< Correlations, local uv coordinates*/
    TH2F* m_correlationsSVD1PXD2[2];    /**< Correlations, local uv coordinates*/
    //TH2F* m_correlationsTel3SVD1SP[2];  /**< Correlations, global yz coordinates*/
    //TH2F* m_correlationsTel4SVD4SP[2];  /**< Correlations, global yz coordinates*/
    //TH2F* m_correlationsTel3PXD2SP[2];  /**< Correlations, global yz coordinates*/
    //TH2F* m_correlationsSVD1PXD2SP[2];  /**< Correlations, global yz coordinates*/
  };

  inline const SVD::SensorInfo& TelxVXDModule::getInfoSVD(int index) const
  {
    // for only index: 5,6,7,8.
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane - 3, 1, iPlane - 3);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const PXD::SensorInfo& TelxVXDModule::getInfoPXD(int index) const
  {
    // for only index: 3,4.
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane - 3, 1, iPlane - 3);
    return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

  inline const TEL::SensorInfo& TelxVXDModule::getInfoTel(int index) const
  {
    // for only index: 0,1,2,9,10,11.
    int iPlane = indexToPlane(index);
    VxdID sensorID;
    if (iPlane <= 3)
      sensorID = VxdID(7 , 2, iPlane);
    else if (iPlane >= 10)
      sensorID = VxdID(7 , 3, iPlane - 6);
    return dynamic_cast<const TEL::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }

}
#endif

