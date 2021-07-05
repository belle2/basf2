/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
