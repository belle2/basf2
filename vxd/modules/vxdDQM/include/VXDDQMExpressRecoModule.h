/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** VXD DQM Module */
  class VXDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    VXDDQMExpressRecoModule();
    /* Destructor */
    virtual ~VXDDQMExpressRecoModule();

    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** flag <0,1> for using digits only, no clusters will be required, default = 0 */
    int m_UseDigits = 0;
    /** flag <0,1> very special case for swap of u-v coordinates */
    int m_SwapPXD = 0;
    /** set granulation of histogram plots, default is 1 deg (1 mm), min = 0.02, max = 5.0 */
    float m_CorrelationGranulation = 1.0;

    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** SVDShaperDigits StoreArray name */
    std::string m_storeSVDShaperDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;
    /** SVDClusters StoreArray name */
    std::string m_storeSVDClustersName;
    /** PXDClustersToPXDDigits RelationArray name */
    std::string m_relPXDClusterDigitName;
    /** SVDClustersToSVDDigits RelationArray name */
    std::string m_relSVDClusterDigitName;

    /** Cut threshold of PXD signal for accepting to correlations, default = 0 ADU */
    float m_CutCorrelationSigPXD = 0;
    /** Cut threshold of SVD signal for accepting to correlations in u, default = 0 ADU */
    float m_CutCorrelationSigUSVD = 0;
    /** Cut threshold of SVD signal for accepting to correlations in v, default = 0 ADU */
    float m_CutCorrelationSigVSVD = 0;
    /** Cut threshold of SVD time window for accepting to correlations, default = 70 ns */
    float m_CutCorrelationTimeSVD = 70;

    /** Correlations and hit maps from space points */
    TH2F** m_correlationsSP;
    /** Correlations and hit maps from space points - differencies in Phi*/
    TH1F** m_correlationsSP1DPhi;
    /** Correlations and hit maps from space points - differencies in Theta*/
    TH1F** m_correlationsSP1DTheta;

  };

}

