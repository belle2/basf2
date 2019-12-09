#include <analysis/dataobjects/DistanceTools.h>
#include <framework/logging/Logger.h>
#include "TMath.h"

using namespace Belle2;

TVector3 DistanceTools::trackToVtxVec(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos)
{
  TVector3 trackDir((1. / trackP.Mag()) * trackP);
  TVector3 r(vtxPos - trackPos);
  return (r.Dot(trackDir)) * trackDir - r;
}

double DistanceTools::trackToVtxDist(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos)
{
  return trackToVtxVec(trackPos, trackP, vtxPos).Mag();
}

TMatrixDSym DistanceTools::trackToVtxCovmat(TVector3 const& trackP,
                                            TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat)
{
  if (trackPosCovMat.GetNcols() != 3 || vtxPosCovMat.GetNcols() != 3) {
    B2ERROR("in DistanceTools::trackToVtxCovmat, matrices must be of size 3");
    //trackPosCovMat.Print();
    //vtxPosCovMat.Print();
    return TMatrixDSym(3);
  }

  TMatrixDSym rCovMat(trackPosCovMat + vtxPosCovMat);
  TVector3 trackDir((1. / trackP.Mag()) * trackP);
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

double DistanceTools::trackToVtxDistErr(TVector3 const& trackPos, TVector3 const& trackP, TVector3 const& vtxPos,
                                        TMatrixDSym const& trackPosCovMat, TMatrixDSym const& vtxPosCovMat)
{
  TMatrixDSym covMat(trackToVtxCovmat(trackP, trackPosCovMat, vtxPosCovMat));
  TVector3 dVec(trackToVtxVec(trackPos, trackP, vtxPos));
  // n is the normalise vector in the direction of the POCA between the track and the vtx
  TVector3 n((1. / dVec.Mag()) * dVec);

  double ret(0);

  //error on the distance computed as d^T * covMat * d
  for (int i(0); i < 3; ++i)
    for (int j(0); j < 3; ++j)
      ret += n(i) * covMat(i, j) * n(j);

  return TMath::Sqrt(ret);
}


TVector3 DistanceTools::vtxToVtxVec(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos)
{
  return vtx2Pos - vtx1Pos;
}

double DistanceTools::vtxToVtxDist(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos)
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

double DistanceTools::vtxToVtxDistErr(TVector3 const& vtx1Pos, TVector3 const& vtx2Pos,
                                      TMatrixDSym const& vtx1CovMat, TMatrixDSym const& vtx2CovMat)
{
  TMatrixDSym covMat(vtxToVtxCovMat(vtx1CovMat, vtx2CovMat));
  TVector3 dVec(vtxToVtxVec(vtx1Pos, vtx2Pos));
  TVector3 n((1. / dVec.Mag()) * dVec);

  double ret(0);

  //error on the distance computed as d^T * covMat * d
  for (int i(0); i < 3; ++i)
    for (int j(0); j < 3; ++j)
      ret += n(i) * covMat(i, j) * n(j);

  return TMath::Sqrt(ret);
}

DistanceTools::DistanceTools() = default;
