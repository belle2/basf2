/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCKarimakiFitter.h"

#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCKarimakiFitter)


namespace {
  CDCKarimakiFitter* g_fitter = nullptr;
  //CDCKarimakiFitter* g_lineFitter = nullptr;
  //CDCKarimakiFitter* g_originCircleFitter = nullptr;
}



const CDCKarimakiFitter& CDCKarimakiFitter::getFitter()
{
  if (not g_fitter) {
    g_fitter = new CDCKarimakiFitter();
  }
  return *g_fitter;
}



// const CDCKarimakiFitter& CDCKarimakiFitter::getLineFitter()
// {
//   if (not g_lineFitter) {
//     g_lineFitter = new CDCKarimakiFitter();
//     g_lineFitter->setLineConstrained();
//   }
//   return *g_lineFitter;
// }



// const CDCKarimakiFitter& CDCKarimakiFitter::getOriginCircleFitter()
// {
//   if (not g_originCircleFitter) {
//     g_originCircleFitter = new CDCKarimakiFitter();
//     g_originCircleFitter->setOriginConstrained();
//   }
//   return *g_originCircleFitter;
// }


CDCKarimakiFitter::CDCKarimakiFitter()
{

}

CDCKarimakiFitter::~CDCKarimakiFitter()
{

}
