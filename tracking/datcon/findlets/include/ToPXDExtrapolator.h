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
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;
  class VxdID;
  class PXDIntercept;

  /**
   * Findlet to extrapolate found tracks to the PXD sensors and calculate intercepts.
   */
  class ToPXDExtrapolator : public TrackFindingCDC::Findlet<const std::pair<double, double>, const std::pair<double, double>,
    std::pair<VxdID, long>, std::pair<VxdID, long>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const std::pair<double, double>, const std::pair<double, double>,
          std::pair<VxdID, long>, std::pair<VxdID, long>>;

  public:
    /// Find intercepts in the 2D Hough space
    ToPXDExtrapolator();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create tracks for extrapolation to PXD
    void apply(const std::vector<std::pair<double, double>>& uTracks, const std::vector<std::pair<double, double>>& vTracks,
               std::vector<std::pair<VxdID, long>>& uExtrapolations, std::vector<std::pair<VxdID, long>>& vExtrapolations) override;

  private:
    /// extrapolate the u-track to the two PXD layers
    /// @param trackPhi azimtutal angle of the track
    /// @param trackRadius radius of the track
    /// @param layer PXD layer to extrapolate to
    /// @param uExtrapolations vector containing the extrapolated positions in u
    void extrapolateUTrack(const double trackPhi, const double trackRadius, const uint layer,
                           std::vector<std::pair<VxdID, long>>& uExtrapolations);

    /// extrapolate the v-track to the two PXD layers
    /// @param tanLambda azimtutal angle of the track
    /// @param layer PXD layer to extrapolate to
    /// @param uExtrapolations vector containing the extrapolated positions in u
    void extrapolateVTrack(const long tanLambda, const uint layer,
                           std::vector<std::pair<VxdID, long>>& vExtrapolations);

    /// Create ROIs in phi only if the absolute difference in phi between sensor and track is smaller than this value on L1.
    double m_param_phiCutL1 = M_PI / 3.;
    /// Create ROIs in phi only if the absolute difference in phi between sensor and track is smaller than this value on L2.
    double m_param_phiCutL2 = M_PI / 4.;

    /// Create PXDIntercepts?
    bool m_param_createPXDIntercepts = false;
    /// name of the PXDIntercept StoreArray
    std::string m_param_PXDInterceptStoreArrayName = "DATCONFPGAPXDIntercepts";
    /// PXDIntercept StoreArray
    StoreArray<PXDIntercept> m_pxdIntercepts;

    // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
    /// shift of the sensor center along z for L1, in µm
    /// for use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ
    const long centerZShiftLayer1[2] = {36825500, -8825500};
    /// shift of the sensor center along z for L2, in µm
    /// for use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ
    const long centerZShiftLayer2[2] = {50145500, -12145500};
    /// radius of L1 and L2, in µm
    const long layerRadius[2] = {14285, 22121};
    /// length of the modules flr L1 and L2, in µm
    const long sensorLength[2] = {44800000, 61440000};
    /// minimum y coordinate for a ladder in the position of ladder 1 (perpendicular to the x-axis), in µm
    const long sensorMinY  = -3600000;
    /// maximum y coordinate for a ladder in the position of ladder 1 (perpendicular to the x-axis), in µm
    const long sensorMaxY  = 8900000;
    /// shift of the sensor center in r-phi
    const long shiftY      = (sensorMaxY + sensorMinY) / 2;

    /// number of ladders per layer
    const uint laddersPerLayer[2] = {8, 12};

  };
}
