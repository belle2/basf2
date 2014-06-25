/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRiemannFitter.h"

#include <Eigen/Dense>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRiemannFitter)


namespace {
  CDCRiemannFitter* g_fitter = nullptr;
  CDCRiemannFitter* g_lineFitter = nullptr;
  CDCRiemannFitter* g_originCircleFitter = nullptr;
}



const CDCRiemannFitter& CDCRiemannFitter::getFitter()
{
  if (not g_fitter) {
    g_fitter = new CDCRiemannFitter();
  }
  return *g_fitter;
}



const CDCRiemannFitter& CDCRiemannFitter::getLineFitter()
{
  if (not g_lineFitter) {
    g_lineFitter = new CDCRiemannFitter();
    g_lineFitter->setLineConstrained();
  }
  return *g_lineFitter;
}



const CDCRiemannFitter& CDCRiemannFitter::getOriginCircleFitter()
{
  if (not g_originCircleFitter) {
    g_originCircleFitter = new CDCRiemannFitter();
    g_originCircleFitter->setOriginConstrained();
  }
  return *g_originCircleFitter;
}



CDCRiemannFitter::CDCRiemannFitter()
{
}



CDCRiemannFitter::~CDCRiemannFitter()
{
}

