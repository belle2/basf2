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

#include <cdc/dataobjects/WireID.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCLayerAlignment.h>

#include <alignment/GlobalLabel.h>
#include <alignment/GlobalDerivatives.h>
#include <alignment/Hierarchy.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace alignment;

bool AlignableCDCRecoHit::s_enableEventT0LocalDerivative = true;

std::pair<std::vector<int>, TMatrixD> AlignableCDCRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  GlobalDerivatives globals;

  // CDC Calibration -------------------------------------------------

  // T0 calibration (per wire) TODO check sign!!!
  globals.add(
    GlobalLabel::construct<CDCTimeZeros>(getWireID(), 0),
    -1. * double(int(m_leftRight))
  );

  // Time-walk calibration (per board) TODO checksign!!!
  globals.add(
    GlobalLabel::construct<CDCTimeWalks>(CDCGeometryPar::Instance().getBoardID(getWireID()), 0),
    -1. * sqrt(m_adcCount) * double(int(m_leftRight))
  );

  // CDC Alignment ---------------------------------------------------

  auto tdir = sop->getDir();
  auto ndir = sop->getPlane()->getNormal();
  auto udir = sop->getPlane()->getU();
  auto vdir = sop->getPlane()->getV();
  auto pos  = sop->getPos();

  auto tn = tdir[0] * ndir[0] + tdir[1] * ndir[1] + tdir[2] * ndir [2];
  // d residual / d measurement
  auto drdm = TMatrixD(3, 3);
  drdm.UnitMatrix();
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      drdm(i, j) -= tdir[i] * ndir[j] / tn;
    }
  }
  // d measurement / d global rigid body param
  auto dmdg = TMatrixD(3, 6);
  dmdg.Zero();
  dmdg[0][0] = 1.; dmdg[0][4] = -pos[2]; dmdg[0][5] =  pos[1];
  dmdg[1][1] = 1.; dmdg[1][3] =  pos[2]; dmdg[1][5] = -pos[0];
  dmdg[2][2] = 1.; dmdg[2][3] = -pos[1]; dmdg[2][4] =  pos[0];
  // d local residual / d global residual
  auto drldrg = TMatrixD(3, 3);
  for (int i = 0; i < 3; ++i) {
    drldrg(0, i) = udir[i];
    drldrg(1, i) = vdir[i];
    drldrg(2, i) = ndir[i];
  }
  // d local residual / global rigid body param
  auto drldg = drldrg * (drdm * dmdg);

  // Alignment of layer X
  globals.add(
    GlobalLabel::construct<CDCLayerAlignment>(getWireID().getICLayer(), CDCLayerAlignment::layerX),
    drldg(0, 0)
  );

  // Alignment of layer Y
  globals.add(
    GlobalLabel::construct<CDCLayerAlignment>(getWireID().getICLayer(), CDCLayerAlignment::layerY),
    drldg(0, 1)
  );

  // Alignment of layer rotation (gamma)
  globals.add(
    GlobalLabel::construct<CDCLayerAlignment>(getWireID().getICLayer(), CDCLayerAlignment::layerPhi),
    drldg(0, 5)
  );
  //
  /**
  // Alignment of wires X in global coords
  globals.add(
    GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), 1),
    drldg(0, 0)
  );

  // Alignment of wires Y in global coords
  globals.add(
    GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), 2),
    drldg(0, 1)
  );

  // Alignment of wires rotation (gamma) in global coords
  globals.add(
    GlobalLabel::construct<CDCAlignment>(getWireID().getEWire(), 6),
    drldg(0, 5)
  );
  */

  /**
  // Gravitational wire sagging TODO highly experimental
  double zWire; //TODO what is this and how to get?
  double zWireM;//TODO what is this and how to get?
  double zWireP;//TODO what is this and how to get?
  double zRel = max(0., min(1., (zWire - zWireM) / (zWireP - zWireM)));

  globals.add(
    GlobalLabel::construct<CDCAlignment>(getWireID(), CDCAlignment::wireTension),
    drldg(0, 1) * 4.0 * zRel * (1.0 - zRel)
  );
  */

  return globals;
}


TMatrixD AlignableCDCRecoHit::localDerivatives(const genfit::StateOnPlane*)
{
  if (!s_enableEventT0LocalDerivative)
    return TMatrixD();

  // CDC track time correction ----------------------------------------
  //TODO change to derivative of the full Xt relation
  double driftVelocity = CDCGeometryPar::Instance().getNominalDriftV();

  TMatrixD locals(2, 1);
  //TODO sign: plus or minus??
  locals(0, 0) = - double(int(m_leftRight)) * driftVelocity;
  locals(1, 0) = 0.; // insesitive coordinate along wire

  return locals;
}
