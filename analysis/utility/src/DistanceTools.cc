/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/utility/DistanceTools.h>
#include <framework/logging/Logger.h>
#include "TMath.h"

using namespace Belle2;


ROOT::Math::XYZVector DistanceTools::poca(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                                          ROOT::Math::XYZVector const& vtxPos)
{
  ROOT::Math::XYZVector trackDir(trackP.Unit());
  ROOT::Math::XYZVector r(vtxPos - trackPos);
  return trackPos + r.Dot(trackDir) * trackDir;
}

ROOT::Math::XYZVector DistanceTools::trackToVtxVec(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                                                   ROOT::Math::XYZVector const& vtxPos)
{
  ROOT::Math::XYZVector trackDir(trackP.Unit());
  ROOT::Math::XYZVector r(vtxPos - trackPos);
  return r - (r.Dot(trackDir)) * trackDir;
}

double DistanceTools::trackToVtxDist(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                                     ROOT::Math::XYZVector const& vtxPos)
{
  return trackToVtxVec(trackPos, trackP, vtxPos).R();
}

TMatrixDSym DistanceTools::trackToVtxCovmat(ROOT::Math::XYZVector const& trackP,
                                            TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat)
{
  if (trackPosCovMat.GetNcols() != 3 || vtxPosCovMat.GetNcols() != 3) {
    B2ERROR("in DistanceTools::trackToVtxCovmat, matrices must be of size 3");
    return TMatrixDSym(3);
  }

  TMatrixDSym rCovMat(trackPosCovMat + vtxPosCovMat);
  ROOT::Math::XYZVector trackDir(trackP.Unit());
  //d_j = r_j - v_j * v_k r_k
  //Jij = del_i d_j = delta_ij - v_i * v_j
  //Since the vector of closest approach is a linear function of r, its
  //propagation of errors is exact
  TMatrixDSym Jacobian(3);
  // Jacobian_ij = delta_ij -v(i)v(j)
  for (int i(0); i < 3; ++i) {
    for (int j(0); j < 3; ++j) {
      double trackDir_i = (i == 0) ? trackDir.X() : (i == 1) ? trackDir.Y() : trackDir.Z();
      double trackDir_j = (j == 0) ? trackDir.X() : (j == 1) ? trackDir.Y() : trackDir.Z();
      Jacobian(i, j) = -trackDir_i * trackDir_j;
    }
  }
  for (int i(0); i < 3; ++i)
    Jacobian(i, i) += 1;

  return rCovMat.Similarity(Jacobian);  //calculates J * rCovMat * J^T, and returns it

}

double DistanceTools::trackToVtxDistErr(ROOT::Math::XYZVector const& trackPos, ROOT::Math::XYZVector const& trackP,
                                        ROOT::Math::XYZVector const& vtxPos,
                                        TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat)
{
  TMatrixDSym covMat(trackToVtxCovmat(trackP, trackPosCovMat, vtxPosCovMat));
  ROOT::Math::XYZVector dVec(trackToVtxVec(trackPos, trackP, vtxPos));
  // n is the normalise vector in the direction of the POCA between the track and the vtx
  ROOT::Math::XYZVector n((1. / dVec.R()) * dVec);

  double ret(0);

  //error on the distance computed as d^T * covMat * d
  for (int i(0); i < 3; ++i) {
    for (int j(0); j < 3; ++j) {
      double n_i = (i == 0) ? n.X() : (i == 1) ? n.Y() : n.Z();
      double n_j = (j == 0) ? n.X() : (j == 1) ? n.Y() : n.Z();
      ret += n_i * covMat(i, j) * n_j;
    }
  }

  return TMath::Sqrt(ret);
}


ROOT::Math::XYZVector DistanceTools::vtxToVtxVec(ROOT::Math::XYZVector const& vtx1Pos, ROOT::Math::XYZVector const& vtx2Pos)
{
  return vtx2Pos - vtx1Pos;
}

double DistanceTools::vtxToVtxDist(ROOT::Math::XYZVector const& vtx1Pos, ROOT::Math::XYZVector const& vtx2Pos)
{
  return vtxToVtxVec(vtx1Pos, vtx2Pos).R();
}

TMatrixDSym DistanceTools::vtxToVtxCovMat(TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat)
{
  if (vtx1CovMat.GetNcols() != 3 || vtx2CovMat.GetNcols() != 3) {
    B2ERROR("in DistanceTools::vtxToVtxCovMat, matrices must be of size 3");
    return TMatrixDSym(3);
  }

  return vtx1CovMat + vtx2CovMat;
}

double DistanceTools::vtxToVtxDistErr(ROOT::Math::XYZVector const& vtx1Pos, ROOT::Math::XYZVector const& vtx2Pos,
                                      TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat)
{
  TMatrixDSym covMat(vtxToVtxCovMat(vtx1CovMat, vtx2CovMat));
  ROOT::Math::XYZVector dVec(vtxToVtxVec(vtx1Pos, vtx2Pos));
  ROOT::Math::XYZVector n((1. / dVec.R()) * dVec);

  double ret(0);

  //error on the distance computed as d^T * covMat * d
  for (int i(0); i < 3; ++i) {
    for (int j(0); j < 3; ++j) {
      double n_i = (i == 0) ? n.X() : (i == 1) ? n.Y() : n.Z();
      double n_j = (j == 0) ? n.X() : (j == 1) ? n.Y() : n.Z();
      ret += n_i * covMat(i, j) * n_j;
    }
  }

  return TMath::Sqrt(ret);
}

