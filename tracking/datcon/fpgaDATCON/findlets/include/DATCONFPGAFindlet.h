/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/datcon/fpgaDATCON/findlets/SVDShaperDigitConverter.h>
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterizer.h>
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterLoaderAndPreparer.h>
#include <tracking/datcon/fpgaDATCON/findlets/FastInterceptFinder2DFPGA.h>
#include <tracking/datcon/fpgaDATCON/findlets/ToPXDExtrapolator.h>
#include <tracking/datcon/fpgaDATCON/findlets/ROICalculator.h>
#include <tracking/datcon/fpgaDATCON/entities/DATCONSVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class VxdID;

  /**
   * Findlet for performing the DATCON ROI calculation close to the implementation
   * on FPGA.
   */
  class DATCONFPGAFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor for adding the subfindlets
    DATCONFPGAFindlet();

    /// Default desctructor
    ~DATCONFPGAFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Function to call all the sub-findlets
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    /// Findlets:
    /// Convert SVDShaperDigits into DATCONSVDDigits
    SVDShaperDigitConverter m_digitConverter;

    /// Cluster u-side strips
    DATCONSVDClusterizer m_uClusterizer;
    /// Cluster v-side strips
    DATCONSVDClusterizer m_vClusterizer;

    /// Load DATCON SVDCluster (still the class is just a SVDCluster) created by the DATCONSVDClusterizer
    /// and calculate the quantities used in the Hough Trafo (intercept finding)
    DATCONSVDClusterLoaderAndPreparer m_clusterLoaderAndPreparer;

    /// Hough Space intercept finding for u-side
    FastInterceptFinder2DFPGA m_uInterceptFinder;
    /// Hough Space intercept finding for v-side
    FastInterceptFinder2DFPGA m_vInterceptFinder;

    /// Extrapolate found tracks to PXD sensors
    ToPXDExtrapolator m_toPXDExtrapolator;

    /// Calculate and store ROIs
    ROICalculator m_ROICalculator;


    // Vectors containing the information throughout the steps and sub findlets
    /// vector containing u-side DATCONSVDDigits only
    std::vector<DATCONSVDDigit> m_uDigits;
    /// vector containing v-side DATCONSVDDigits only
    std::vector<DATCONSVDDigit> m_vDigits;

    /// vector containing u-side SVDClusters only
    std::vector<SVDCluster> m_uClusters;
    /// vector containing v-side SVDClusters only
    std::vector<SVDCluster> m_vClusters;

    /// vector containing the prepared u-side hits for intercept finding
    std::vector<std::pair<VxdID, std::pair<long, long>>> m_uHits;
    /// vector containing the prepared v-side hits for intercept finding
    std::vector<std::pair<VxdID, std::pair<long, long>>> m_vHits;

    /// u-side "tracks" from intercept finding, consisting of the x-y pair
    /// from the intercept in the 2D Hough Space
    std::vector<std::pair<double, double>> m_uTracks;
    /// v-side "tracks" from intercept finding, consisting of the x-y pair
    /// from the intercept in the 2D Hough Space
    std::vector<std::pair<double, double>> m_vTracks;

    /// Extrapolated hits in u direction
    std::vector<std::pair<VxdID, long>> m_uExtrapolations;
    /// Extrapolated hits in v direction
    std::vector<std::pair<VxdID, long>> m_vExtrapolations;

  };
}
