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

using namespace std;
using namespace Belle2;
using namespace CDC;

std::vector< int > AlignableCDCRecoHit::labels()
{
  // do nothing for stereo
  if (getWireID().getISuperLayer() % 2 == 1)
    return std::vector< int >();

  // Invalid wire == all wires in layer
  WireID layer(getWireID().getISuperLayer(), getWireID().getILayer(), 511);
  std::vector< int > indices;
  indices.push_back(GlobalLabel(layer, 1));
  indices.push_back(GlobalLabel(layer, 2));
  //indices.push_back(GlobalLabel(layer, 9));
  //indices.push_back(int((8192-m_tdcCount)));

  return indices;
}

TMatrixD AlignableCDCRecoHit::derivatives(const genfit::StateOnPlane* sop)
{
  if (getWireID().getISuperLayer() % 2 == 1)
    return TMatrixD();

  //double z = sop->getPos().Z();
  //const TVector3& p = sop->getMom();
  //const double wx = sop->getPlane()->getO().x();
  //const double wy = sop->getPlane()->getO().y();
  //const double px = p.x();
  //const double py = p.y();
  //const double cross = wx * py - wy * px;
  //const double dot   = wx * px + wy * py;
  //double alpha = atan2(cross, dot);
  //double theta = atan2(p.Perp(), p.z());

  //double trackTime = sop->getTime();

  //double driftLength = s_tdcCountTranslator->getDriftLength(m_tdcCount, getWireID(), trackTime, m_leftRight, z, alpha, theta);

  TVector3 mom = sop->getMom();
  double phi = atan2(mom[1], mom[0]);
  //double driftTime = driftLength / 4.e-3; // 1 Unit in the TDC count equals 1 ns

  //if (double(8192-m_tdcCount) < 30.)
  //  return TMatrixD();

  TMatrixD matrix(2, 2);
  //TMatrixD matrix(2, 1);

  matrix(0, 0) = - sin(phi); // dx
  matrix(0, 1) = cos(phi); // dy
  //matrix(0, 2) = double(int(m_leftRight)) * driftTime; // drift velocity correction [cm/ns]
  //matrix(0, 0) = double(int(m_leftRight)) * driftTime; // drift velocity correction [cm/ns]

  // Second dimension (v) is insensitive
  matrix(1, 0) = 0.;
  matrix(1, 1) = 0.;
  //matrix(1, 2) = 0.;
  return matrix;
}

TMatrixD AlignableCDCRecoHit::localDerivatives(const genfit::StateOnPlane*)
{
  // Later...
  return TMatrixD();

  WireID wire(getWireID());
  // do nothing for stereo
  int isStereo = (wire.getISuperLayer() % 2 == 1);
  if (isStereo)
    return TMatrixD();

  double driftVelocity = 4.e-3;
  TMatrixD locals(2, 1);
  locals(0, 0) = - double(int(m_leftRight)) * driftVelocity;
  locals(1, 0) = 0.;
  return locals;
}

std::vector< int > AlignableCDCRecoHit::localLabels()
{
  WireID wire(getWireID());
  // do nothing for stereo
  int isStereo = (wire.getISuperLayer() % 2 == 1);
  if (isStereo)
    return std::vector< int >();


  std::vector< int > indices;
  indices.push_back(1);
  return indices;
}
