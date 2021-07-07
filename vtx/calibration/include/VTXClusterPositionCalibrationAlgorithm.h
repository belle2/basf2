/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vtx/dbobjects/VTXClusterShapeIndexPar.h>
#include <vtx/dbobjects/VTXClusterPositionEstimatorPar.h>

#include <vector>
#include <set>

namespace Belle2 {
  /**
   * Class implementing the VTX cluster position calibration algorithm
   */
  class  VTXClusterPositionCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to VTXClusterPositionCalibrationAlgorithm
    VTXClusterPositionCalibrationAlgorithm();

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
    virtual EResult calibrate() override;

  private:

    /// Returns a new classifier and index trained on cluster tree
    void createShapeClassifier(std::string treename, VTXClusterShapeClassifierPar* shapeClassifier,
                               VTXClusterShapeIndexPar* shapeIndexer);

    /// Returns a mirrored version of shape classifier
    VTXClusterShapeClassifierPar mirrorShapeClassifier(VTXClusterShapeClassifierPar* shapeClassifier,
                                                       VTXClusterShapeIndexPar* shapeIndexer, int clusterKind);

    /// Returns a shape classifier using global shape indices instead of local ones
    VTXClusterShapeClassifierPar localToGlobal(VTXClusterShapeClassifierPar* localShapeClassifier,
                                               VTXClusterShapeIndexPar* localShapeIndexer, VTXClusterShapeIndexPar* globalShapeIndexer);

    /** Branches for tree */
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
    /** Size in V */
    int m_sizeV;

    /** Branches for pitchtree */
    /** Pitch in V */
    float m_pitchV;
    /** Pitch in V */
    int m_clusterKind;


    /** Helper needed to map the clusterkind to the V pitch of the sensor */
    std::map<int, float> m_pitchMap;
    /** Helper needed to map the name of a shape to the name of the mirrored shape */
    std::map<std::string, std::string> m_mirrorMap;
    /** Helper needed to map the name of a shape to the V size of the cluster */
    std::map<std::string, int> m_sizeMap;
    /** Set of unique shape names */
    std::set<std::string> m_shapeSet;
  };
} // namespace Belle2


