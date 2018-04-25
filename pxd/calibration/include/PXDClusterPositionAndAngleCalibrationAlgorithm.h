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

#include <TFile.h>

namespace Belle2 {
  /**
   * Class implementing the PXD cluster position and angle calibration algorithm
   */
  class  PXDClusterPositionAndAngleCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to PXDClusterPositionAndAngleCalibrationAlgorithm
    PXDClusterPositionAndAngleCalibrationAlgorithm();

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
    void createShapeClassifier(std::string treename, PXDClusterShapeClassifierPar* shapeClassifier,
                               PXDClusterShapeIndexPar* shapeIndexer);

    /// Returns a mirrored version of shape classifier
    PXDClusterShapeClassifierPar mirrorShapeClassifier(PXDClusterShapeClassifierPar* shapeClassifier,
                                                       PXDClusterShapeIndexPar* shapeIndexer, int clusterKind);

    // Returns a shape classifier using global shape indices instead of local ones
    PXDClusterShapeClassifierPar localToGlobal(PXDClusterShapeClassifierPar* localShapeClassifier,
                                               PXDClusterShapeIndexPar* localShapeIndexer, PXDClusterShapeIndexPar* globalShapeIndexer);

    /** Branches for tree */
    /** Name of cluster shape */
    std::string m_shapeName;
    /** Name of mirrored cluster shape */
    std::string m_mirroredShapeName;
    /** Eta value of cluster */
    float m_clusterEta;
    /** Charge value of cluster */
    float m_clusterCharge;
    /** Position offset u of cluster */
    float m_positionOffsetU;
    /** Position offset v of cluster */
    float m_positionOffsetV;
    /** Incidence angle thetaU of truehit related to cluster */
    float m_thetaU;
    /** Incidence angle thetaV of truehit related to cluster */
    float m_thetaV;
    /** Magnitude of momentum of truehit related to cluster */
    float m_momentumMag;
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

    TFile* m_rootFile;
  };
} // namespace Belle2


