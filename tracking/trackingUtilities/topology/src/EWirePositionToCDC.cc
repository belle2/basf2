/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
