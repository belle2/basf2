/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/EWirePositionToCDC.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDC::CDCGeometryPar::EWirePosition TrackFindingCDC::toCDC(EWirePosition wirePosition)
{
  if (wirePosition == EWirePosition::c_Base) {
    return CDC::CDCGeometryPar::EWirePosition::c_Base;
  } else if (wirePosition == EWirePosition::c_Misaligned) {
    return CDC::CDCGeometryPar::EWirePosition::c_Misaligned;
  } else if (wirePosition == EWirePosition::c_Aligned) {
    return CDC::CDCGeometryPar::EWirePosition::c_Aligned;
  }
  B2ERROR("Unexpected EWirePosition");
  return CDC::CDCGeometryPar::EWirePosition::c_Base;
}

EWirePosition TrackFindingCDC::fromCDC(CDC::CDCGeometryPar::EWirePosition wirePosSet)
{
  if (wirePosSet == CDC::CDCGeometryPar::EWirePosition::c_Base) {
    return EWirePosition::c_Base;
  } else if (wirePosSet == CDC::CDCGeometryPar::EWirePosition::c_Misaligned) {
    return EWirePosition::c_Misaligned;
  } else if (wirePosSet == CDC::CDCGeometryPar::EWirePosition::c_Aligned) {
    return EWirePosition::c_Aligned;
  }
  B2ERROR("Unexpected EWirePosition");
  return EWirePosition::c_Base;
}
