/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDQMMODULE_H_
#define SVDDQMMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class SVDDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMModule();
    /* Destructor */
    virtual ~SVDDQMModule();

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

    float m_CutSVDCharge = 22.0;           /**< cut for accepting to hitmap histogram, using strips only, default = 22 */
    int m_UseStrips = 0;                   /**< flag <0,1> for using strips only, no clusters will be required, default = 0 */
    int m_SaveOtherHistos = 1;             /**< flag <0,1> for creation of more plots for experts mostly, default = 1 */

    std::string m_storeSVDDigitsName;      /**< SVDDigits StoreArray name */
    std::string m_storeSVDClustersName;    /**< SVDClusters StoreArray name */
    std::string m_relSVDClusterDigitName;  /**< SVDClustersToSVDDigits RelationArray name */

    TH1F** m_firedU;         /**< Fired u strips per event */
    TH1F** m_firedV;         /**< Fired v strips per event */
    TH1F** m_clustersU;      /**< u clusters per event */
    TH1F** m_clustersV;      /**< v clusters per event */
    TH2F** m_hitMapU;        /**< Hitmaps strips for u */
    TH2F** m_hitMapV;        /**< Hitmaps strips for v */
    TH1F** m_hitMapUCl;      /**< Hitmaps clusters for u */
    TH1F** m_hitMapVCl;      /**< Hitmaps clusters for v */
    TH1F** m_chargeU;        /**< u charge of clusters */
    TH1F** m_chargeV;        /**< v charge of clusters */
    TH1F** m_chargeUSt;      /**< u charge of strips */
    TH1F** m_chargeVSt;      /**< v charge of strips */
    TH1F** m_seedU;          /**< u seed */
    TH1F** m_seedV;          /**< v seed */
    TH1F** m_sizeU;          /**< u size */
    TH1F** m_sizeV;          /**< v size */
    TH1F** m_timeU;          /**< u time */
    TH1F** m_timeV;          /**< v time */

    int c_nVXDLayers;                /**< Number of VXD layers on Belle II */
    int c_nPXDLayers;                /**< Number of PXD layers on Belle II */
    int c_nSVDLayers;                /**< Number of SVD layers on Belle II */
    int c_firstVXDLayer;             /**< First VXD layer on Belle II */
    int c_lastVXDLayer;              /**< Last VXD layer on Belle II */
    int c_firstPXDLayer;             /**< First PXD layer on Belle II */
    int c_lastPXDLayer;              /**< Last PXD layer on Belle II */
    int c_firstSVDLayer;             /**< First SVD layer on Belle II */
    int c_lastSVDLayer;              /**< Last SVD layer on Belle II */
    int c_nSVDSensors;               /**< Number of SVD sensors on Belle II */

    /**< Function return index of sensor in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @return Index of sensor in plots.
       */
    int getSensorIndex(int Layer, int Ladder, int Sensor);
    /**< Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor);

  };

}
#endif

