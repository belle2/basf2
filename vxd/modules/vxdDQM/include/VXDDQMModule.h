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

#ifndef VXDDQMMODULE_H_
#define VXDDQMMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class VXDDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    VXDDQMModule();
    /* Destructor */
    virtual ~VXDDQMModule();

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

    int m_UseDigits = 0;                   /**< flag <0,1> for using digits only, no clusters will be required, default = 0 */
    int m_Reduce1DCorrelHistos = 0;        /**< flag <0,1> for removing of 1D correlation plots from output */
    int m_Reduce2DCorrelHistos = 0;        /**< flag <0,1> for removing of 2D correlation plots from output */
    int m_Only23LayersHistos = 0;          /**< flag <0,1> for create only correlation plots for layer 2-3 (PXD-SVD) */
    int m_SaveOtherHistos = 0;             /**< flag <0,1> for creation of correlation plots for non-neighboar layers */
    int m_SwapPXD = 0;                     /**< flag <0,1> very special case for swap of u-v coordinates */
    float m_CorrelationGranulation = 1.0;  /**< set granulation of histogram plots, default is 1 mm, min = 0.02 mm, max = 1 mm */
    int m_IsTB = 0;                        /**< flag <0,1> for using for testbeam (paralel particles in x direction), default = 0 */

    std::string m_storePXDDigitsName;      /**< PXDDigits StoreArray name */
    std::string m_storeSVDDigitsName;      /**< SVDDigits StoreArray name */
    std::string m_storePXDClustersName;    /**< PXDClusters StoreArray name */
    std::string m_storeSVDClustersName;    /**< SVDClusters StoreArray name */
    std::string m_relPXDClusterDigitName;  /**< PXDClustersToPXDDigits RelationArray name */
    std::string m_relSVDClusterDigitName;  /**< SVDClustersToSVDDigits RelationArray name */

    int c_nVXDLayers;                /**< Number of VXD layers on Belle II */
    int c_nPXDLayers;                /**< Number of PXD layers on Belle II */
    int c_nSVDLayers;                /**< Number of SVD layers on Belle II */
    int c_firstVXDLayer;             /**< First VXD layer on Belle II */
    int c_lastVXDLayer;              /**< Last VXD layer on Belle II */
    int c_firstPXDLayer;             /**< First PXD layer on Belle II */
    int c_lastPXDLayer;              /**< Last PXD layer on Belle II */
    int c_firstSVDLayer;             /**< First SVD layer on Belle II */
    int c_lastSVDLayer;              /**< Last SVD layer on Belle II */
    unsigned int c_MaxLaddersInPXDLayer;      /**< Maximum No of PXD ladders on layer */
    unsigned int c_MaxLaddersInSVDLayer;      /**< Maximum No of SVD ladders on layer */
    unsigned int c_MaxSensorsInPXDLayer;      /**< Maximum No of PXD sensors on layer */
    unsigned int c_MaxSensorsInSVDLayer;      /**< Maximum No of SVD sensors on layer */

    float m_CutCorrelationSigPXD = 0;    /**< Cut threshold of PXD signal for accepting to correlations, default = 0 ADU */
    float m_CutCorrelationSigUSVD = 0;   /**< Cut threshold of SVD signal for accepting to correlations in u, default = 0 ADU */
    float m_CutCorrelationSigVSVD = 0;   /**< Cut threshold of SVD signal for accepting to correlations in v, default = 0 ADU */
    float m_CutCorrelationTimeSVD = 70;  /**< Cut threshold of SVD time window for accepting to correlations, default = 70 ns */

    TH2F** m_correlationsSP;         /**< Correlations and hit maps from space points */
    TH1F** m_correlationsSP1DPhi;    /**< Correlations and hit maps from space points - differencies in Phi*/
    TH1F** m_correlationsSP1DTheta;  /**< Correlations and hit maps from space points - differencies in Theta*/

  };

}
#endif

