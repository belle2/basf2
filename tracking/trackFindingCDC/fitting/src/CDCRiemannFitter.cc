/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCFitter2D.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::CDCFitter2D<ExtendedRiemannsMethod>;

CDCRiemannFitter::CDCRiemannFitter() = default;
CDCRiemannFitter::~CDCRiemannFitter() = default;

const CDCRiemannFitter& CDCRiemannFitter::getFitter()
{
  static CDCRiemannFitter fitter;
  return fitter;
}

const CDCRiemannFitter& CDCRiemannFitter::getFitter(bool fromOrigin, bool line)
{
  static CDCRiemannFitter fitter;
  if (fromOrigin) fitter.setOriginConstrained();
  if (line) fitter.setLineConstrained();
  return fitter;
}

const CDCRiemannFitter& CDCRiemannFitter::getLineFitter()
{
  static CDCRiemannFitter lineFitter;
  lineFitter.setLineConstrained();
  return lineFitter;
}

const CDCRiemannFitter& CDCRiemannFitter::getOriginCircleFitter()
{
  static CDCRiemannFitter originCircleFitter;
  originCircleFitter.setOriginConstrained();
  return originCircleFitter;
}
