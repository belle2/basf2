/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thibaud Humair                                           *
 *                                                                        *
 * Inspired from DistanceCalculatorModule by Sourav Dey & Abi Soffer      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <vector>
#include <string>
#include "TVector3.h"
#include "TMatrixDSym.h"

namespace Belle2 {

  class DistanceTools {

  public:

    DistanceTools();

    TVector3 trackToVtxVec(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos);

    double trackToVtxDist(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos);

    TMatrixDSym trackToVtxCovmat(TVector3 const& trackP,
                                 TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat);

    double trackToVtxDistErr(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos,
                             TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat);


    TVector3 vtxToVtxVec(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos);

    double vtxToVtxDist(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos);

    TMatrixDSym vtxToVtxCovMat(TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat);

    double vtxToVtxDistErr(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos,
                           TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat);
  };
}
