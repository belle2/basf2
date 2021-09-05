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


B2Vector3D DistanceTools::poca(B2Vector3D const& trackPos, B2Vector3D const& trackP, B2Vector3D const& vtxPos)
{
  B2Vector3D trackDir(trackP.Unit());
  B2Vector3D r(vtxPos - trackPos);
  return trackPos + r.Dot(trackDir) * trackDir;
}

B2Vector3D DistanceTools::trackToVtxVec(B2Vector3D const& trackPos, B2Vector3D const& trackP, B2Vector3D const& vtxPos)
{
  B2Vector3D trackDir(trackP.Unit());
  B2Vector3D r(vtxPos - trackPos);
  return r - (r.Dot(trackDir)) * trackDir;
}

double DistanceTools::trackToVtxDist(B2Vector3D const& trackPos, B2Vector3D const& trackP, B2Vector3D const& vtxPos)
{
  return trackToVtxVec(trackPos, trackP, vtxPos).Mag();
}

TMatrixDSym DistanceTools::trackToVtxCovmat(B2Vector3D const& trackP,
                                            TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat)
{
  if (trackPosCovMat.GetNcols() != 3 || vtxPosCovMat.GetNcols() != 3) {
    B2ERROR("in DistanceTools::trackToVtxCovmat, matrices must be of size 3");
    return TMatrixDSym(3);
  }

  TMatrixDSym rCovMat(trackPosCovMat + vtxPosCovMat);
  B2Vector3D trackDir(trackP.Unit());
  //d_j = r_j - v_j * v_k r_k
  //Jij = del_i d_j = delta_ij - v_i * v_j
  //Since the vector of closest approach is a linear function of r, its
  //propagation of errors is exact
  TMatrixDSym Jacobian(3);
  // Jacobian_ij = delta_ij -v(i)v(j)
  for (int i(0); i < 3; ++i)
    for (int j(0); j < 3; ++j)
      Jacobian(i, j) = -trackDir(i) * trackDir(j);
  for (int i(0); i < 3; ++i)
    Jacobian(i, i) += 1;

  return rCovMat.Similarity(Jacobian);  //calculates J * rCovMat * J^T, and returns it

}

double DistanceTools::trackToVtxDistErr(B2Vector3D const& trackPos, B2Vector3D const& trackP, B2Vector3D const& vtxPos,
                                        TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat)
{
  TMatrixDSym covMat(trackToVtxCovmat(trackP, trackPosCovMat, vtxPosCovMat));
  B2Vector3D dVec(trackToVtxVec(trackPos, trackP, vtxPos));
  // n is the normalise vector in the direction of the POCA between the track and the vtx
  B2Vector3D n((1. / dVec.Mag()) * dVec);

  double ret(0);

  //error on the distance computed as d^T * covMat * d
  for (int i(0); i < 3; ++i)
    for (int j(0); j < 3; ++j)
      ret += n(i) * covMat(i, j) * n(j);

  return TMath::Sqrt(ret);
}


B2Vector3D DistanceTools::vtxToVtxVec(B2Vector3D const& vtx1Pos, B2Vector3D const& vtx2Pos)
{
  return vtx2Pos - vtx1Pos;
}

double DistanceTools::vtxToVtxDist(B2Vector3D const& vtx1Pos, B2Vector3D const& vtx2Pos)
{
  return vtxToVtxVec(vtx1Pos, vtx2Pos).Mag();
}

TMatrixDSym DistanceTools::vtxToVtxCovMat(TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat)
{
  if (vtx1CovMat.GetNcols() != 3 || vtx2CovMat.GetNcols() != 3) {
    B2ERROR("in DistanceTools::vtxToVtxCovMat, matrices must be of size 3");
    return TMatrixDSym(3);
  }

  return vtx1CovMat + vtx2CovMat;
}

double DistanceTools::vtxToVtxDistErr(B2Vector3D const& vtx1Pos, B2Vector3D const& vtx2Pos,
                                      TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat)
{
  TMatrixDSym covMat(vtxToVtxCovMat(vtx1CovMat, vtx2CovMat));
  B2Vector3D dVec(vtxToVtxVec(vtx1Pos, vtx2Pos));
  B2Vector3D n((1. / dVec.Mag()) * dVec);

  double ret(0);

  //error on the distance computed as d^T * covMat * d
  for (int i(0); i < 3; ++i)
    for (int j(0); j < 3; ++j)
      ret += n(i) * covMat(i, j) * n(j);

  return TMath::Sqrt(ret);
}

