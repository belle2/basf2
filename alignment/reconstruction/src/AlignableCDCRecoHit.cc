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

#include <alignment/Hierarchy.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

std::pair<std::vector<int>, TMatrixD> AlignableCDCRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  std::vector<int> labels;
  TMatrixD derivs(2, 2);

  // TODO/FIXME fix/check this!!
  derivs(0, 0) = 1. * double(int(m_leftRight));
  derivs(1, 0) = 0.; // insensitive coordinate

  //TODO/FIXME fix/check this!
  derivs(0, 1) = sqrt(getCDCHit()->getADCCount());
  derivs(1, 1) = 0.; // insensitive coordinate

  labels.push_back(GlobalLabel::construct<CDCTimeZeros>(getWireID(), 0).label());

  return alignment::GlobalDerivatives::passGlobals({labels, derivs});
}


// TMatrixD AlignableCDCRecoHit::localDerivatives(const genfit::StateOnPlane*)
// {
//   // Later...
//   return TMatrixD();
//
//   WireID wire(getWireID());
//   // do nothing for stereo
//   int isStereo = (wire.getISuperLayer() % 2 == 1);
//   if (isStereo)
//     return TMatrixD();
//
//   double driftVelocity = 4.e-3;
//   TMatrixD locals(2, 1);
//   locals(0, 0) = - double(int(m_leftRight)) * driftVelocity;
//   locals(1, 0) = 0.;
//   return locals;
// }

// std::vector< int > AlignableCDCRecoHit::localLabels()
// {
// //   WireID wire(getWireID());
// //   // do nothing for stereo
// //   int isStereo = (wire.getISuperLayer() % 2 == 1);
// //   if (isStereo)
// //     return std::vector< int >();
//
//
//   std::vector< int > indices;
// //   indices.push_back(1);
//   return indices;
// }
