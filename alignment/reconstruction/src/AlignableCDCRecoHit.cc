/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/reconstruction/AlignableCDCRecoHit.h>

#include <alignment/GlobalLabel.h>
#include <cdc/dataobjects/WireID.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>

#include <alignment/Hierarchy.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

std::pair<std::vector<int>, TMatrixD> AlignableCDCRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  // do nothing for stereo
  //if (getWireID().getISuperLayer() % 2 == 1)
  //  return std::make_pair(std::vector<int>(), TMatrixD());

  std::vector<int> labels;
  TMatrixD derivs(2, 3);
  derivs.Zero();
//
//   // TODO/FIXME fix/check this!!
//   derivs(0, 0) = 1. * double(int(m_leftRight));
//   derivs(1, 0) = 0.; // insensitive coordinate
//
//   labels.push_back(GlobalLabel::construct<CDCTimeZeros>(getWireID(), 0).label());
//
//   //TODO/FIXME fix/check this!
//   derivs(0, 1) = sqrt(getCDCHit()->getADCCount()) * double(int(m_leftRight));
//   derivs(1, 1) = 0.; // insensitive coordinate
//
//   labels.push_back(GlobalLabel::construct<CDCTimeWalks>(CDCGeometryPar::Instance().getBoardID(getWireID()), 0).label());
//
//   //TODO FIXME

  auto tdir = sop->getDir();
  auto ndir = sop->getPlane()->getNormal();
  auto udir = sop->getPlane()->getU();
  auto vdir = sop->getPlane()->getV();

  auto tn = tdir[0] * ndir[0] + tdir[1] * ndir[1] + tdir[2] * ndir [2];
  auto drdm = TMatrixD(3, 3);
  drdm.UnitMatrix();
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      drdm(i, j) -= tdir[i] * ndir[j] / tn;
    }
  }

  auto pos = sop->getPos();

  auto dmdg = TMatrixD(3, 6);
  dmdg.Zero();
  dmdg[0][0] = 1.; dmdg[0][4] = -pos[2]; dmdg[0][5] =  pos[1];
  dmdg[1][1] = 1.; dmdg[1][3] =  pos[2]; dmdg[1][5] = -pos[0];
  dmdg[2][2] = 1.; dmdg[2][3] = -pos[1]; dmdg[2][4] =  pos[0];

  auto drldrg = TMatrixD(3, 3);
  for (int i = 0; i < 3; ++i) {
    drldrg(0, i) = udir[i];
    drldrg(1, i) = vdir[i];
    drldrg(2, i) = ndir[i];
  }

  auto drldg = drldrg * (drdm * dmdg);

  derivs(0, 0) = drldg(0, 0);
  derivs(0, 1) = drldg(0, 1);
  derivs(0, 2) = drldg(0, 5);

  labels.push_back(GlobalLabel::construct<CDCAlignment>(getWireID().getICLayer(), 1).label()); // x shift
  labels.push_back(GlobalLabel::construct<CDCAlignment>(getWireID().getICLayer(), 2).label()); // y shift
  labels.push_back(GlobalLabel::construct<CDCAlignment>(getWireID().getICLayer(), 6).label()); // rotation gamma

  return alignment::GlobalDerivatives::passGlobals({labels, derivs});
}


TMatrixD AlignableCDCRecoHit::localDerivatives(const genfit::StateOnPlane*)
{
  // do nothing for stereo
  //if (getWireID().getISuperLayer() % 2 == 1)
  //  return TMatrixD();

  double driftVelocity = CDCGeometryPar::Instance().getNominalDriftV();
  TMatrixD locals(2, 1);
  locals(0, 0) = - double(int(m_leftRight)) * driftVelocity;
  locals(1, 0) = 0.;
  return locals;
}
