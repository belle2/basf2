/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Benjamin Schwenker                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDClusterShapeIndexPar.h>
#include <pxd/dbobjects/PXDClusterPositionEstimatorPar.h>

#include <vector>
#include <set>

namespace Belle2 {
  /**
   * Class implementing the PXD cluster position calibration algorithm
   */
  class  PXDClusterPositionCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDClusterPositionCalibrationAlgorithm
    PXDClusterPositionCalibrationAlgorithm();

    /// Minimum number of collected clusters for estimating shape likelyhood
    int minClusterForShapeLikelyhood;

    /// Minimum number of collected clusters for estimating cluster position offsets
    int minClusterForPositionOffset;

    /// Maximum number of eta bins for estimating cluster position offsets
    int maxEtaBins;

    /// Vector of clusterkinds to calibrate
    std::vector<int> clusterKinds;

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:

    /// Returns a new classifier and index trained on cluster tree
    void createShapeClassifier(std::string treename, PXDClusterShapeClassifierPar* classifier, PXDClusterShapeIndexPar* index);

    /** Name of cluster shape */
    std::string m_shapeName;
    /** Name of mirrored cluster shape */
    std::string m_mirroredShapeName;
    /** Eta value of cluster */
    float m_clusterEta;
    /** Position offset u of cluster */
    float m_positionOffsetU;
    /** Position offset v of cluster */
    float m_positionOffsetV;

    /** Helper needed to map the name of a shape to the name of the mirrored shape */
    std::map<std::string, std::string> m_mirrorMap;
    std::set<std::string> m_shapeSet;
  };
} // namespace Belle2


