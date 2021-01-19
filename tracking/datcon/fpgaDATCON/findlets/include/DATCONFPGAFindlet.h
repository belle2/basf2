/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/datcon/fpgaDATCON/findlets/SVDShaperDigitConverter.h>
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterizer.h>
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterLoaderAndPreparer.h>
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterLoaderAndPreparer2.h>
#include <tracking/datcon/fpgaDATCON/findlets/FastInterceptFinder2D.h>

#include <svd/dataobjects/SVDCluster.h>

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

#include <tracking/dataobjects/DATCONSVDDigit.h>

namespace Belle2 {
  class ModuleParamList;

  /**
   * Findlet for loading the seeds from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   *
   * If a direction != "invalid" is given, the relations of the tracks to the given store array are checked.
   * If there is a relation with the weight equal to the given direction (meaning there is already a
   * partner for this direction), the track is not passed on.
   */
  class DATCONFPGAFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    DATCONFPGAFindlet();

    /// Default desctructor
    ~DATCONFPGAFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
//     void initialize() override;

    /// Load in the reco tracks and the hits
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    /// Findlets
    SVDShaperDigitConverter m_digitConverter;

    DATCONSVDClusterizer m_uClusterizer;
    DATCONSVDClusterizer m_vClusterizer;

    DATCONSVDClusterLoaderAndPreparer m_clusterLoaderAndPreparer;
    DATCONSVDClusterLoaderAndPreparer2 m_clusterLoaderAndPreparer2;

    FastInterceptFinder2D m_uInterceptFinder;
    FastInterceptFinder2D m_vInterceptFinder;

    std::vector<DATCONSVDDigit> m_uDigits;
    std::vector<DATCONSVDDigit> m_vDigits;

    std::vector<SVDCluster> m_uClusters;
    std::vector<SVDCluster> m_vClusters;

    std::vector<std::pair<VxdID, std::pair<long, long>>> m_uHits;
    std::vector<std::pair<VxdID, std::pair<long, long>>> m_vHits;

    std::vector<std::pair<double, double>> m_uTracks;
    std::vector<std::pair<double, double>> m_vTracks;

  };
}
