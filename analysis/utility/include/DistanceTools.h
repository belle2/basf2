/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "TMatrixDSym.h"
#include <Math/Vector3D.h>

namespace Belle2 {

  /**
   * This namespace contains a collection of function that are useful to compute distances
   * between tracks and vertices.
   *
   * All tracks are assumed to be straight in the current implementation.
   */
  namespace DistanceTools {

    /**
     * Returns the Point Of Closest Approach of a track to a vertex
     */
    ROOT::Math::XYZVector poca(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                               ROOT::Math::XYZVector const& vtxPos);

    /**
     * Returns the 3D vector between a vertex and a track's point of closest approach to that vertex
     */
    ROOT::Math::XYZVector trackToVtxVec(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                                        ROOT::Math::XYZVector const& vtxPos);

    /**
     * Returns the distance between a vertex and a track's point of closest approach to that vertex
     */
    double trackToVtxDist(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                          ROOT::Math::XYZVector const& vtxPos);

    /**
     * Returns the covariance (error) matrix of the 3D vector between a vertex and a track's point
     *  of closest approach to that vertex
     */
    TMatrixDSym trackToVtxCovmat(ROOT::Math::XYZVector const& trackP,
                                 TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat);

    /**
     * Returns the estimated uncertainty between a vertex and a track's point of closest approach
     * to that vertex
     */
    double trackToVtxDistErr(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                             ROOT::Math::XYZVector const& vtxPos,
                             TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat);


    /**
     * Returns the 3D vector between two vertices, ie vtxPos2 - vtxPos1
     */
    ROOT::Math::XYZVector vtxToVtxVec(ROOT::Math::XYZVector const& vtx1Pos, ROOT::Math::XYZVector const& vtx2Pos);

    /**
     * Returns the distance between two vertices
     */
    double vtxToVtxDist(ROOT::Math::XYZVector const& vtx1Pos, ROOT::Math::XYZVector const& vtx2Pos);

    /**
     *  Returns the covariance (error) matrix related to the vector linking two vertices
     */
    TMatrixDSym vtxToVtxCovMat(TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat);

    /**
     * Returns the estimated uncertainty on the distance between two vertices
     */
    double vtxToVtxDistErr(ROOT::Math::XYZVector const& vtx1Pos, ROOT::Math::XYZVector const& vtx2Pos,
                           TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat);
  }
}
