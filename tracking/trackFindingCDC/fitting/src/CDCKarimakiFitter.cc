/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCFitter2D.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::CDCFitter2D<KarimakisMethod>;

CDCKarimakiFitter::CDCKarimakiFitter() = default;
CDCKarimakiFitter::~CDCKarimakiFitter() = default;

const CDCKarimakiFitter& CDCKarimakiFitter::getFitter()
{
  static CDCKarimakiFitter fitter;
  return fitter;
}

const CDCKarimakiFitter& CDCKarimakiFitter::getLineFitter()
{
  static CDCKarimakiFitter lineFitter;
  lineFitter.setLineConstrained();
  return lineFitter;
}

const CDCKarimakiFitter& CDCKarimakiFitter::getNoDriftVarianceFitter()
{
  static CDCKarimakiFitter noDriftVarianceFitter;
  noDriftVarianceFitter.setFitVariance(EFitVariance::c_DriftLength);
  return noDriftVarianceFitter;
}
