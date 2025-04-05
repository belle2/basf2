// /**************************************************************************
//  * basf2 (Belle II Analysis Software Framework)                           *
//  * Author: The Belle II Collaboration                                     *
//  *                                                                        *
//  * See git log for contributors and copyright holders.                    *
//  * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
//  **************************************************************************/
// #include <cdc/topology/EWirePositionToCDC.h>

// using namespace Belle2;
// using namespace CDC;

// CDCGeometryPar::EWirePosition toCDC(EWirePosition wirePosition)
// {
//   if (wirePosition == EWirePosition::c_Base) {
//     return CDCGeometryPar::EWirePosition::c_Base;
//   } else if (wirePosition == EWirePosition::c_Misaligned) {
//     return CDCGeometryPar::EWirePosition::c_Misaligned;
//   } else if (wirePosition == EWirePosition::c_Aligned) {
//     return CDCGeometryPar::EWirePosition::c_Aligned;
//   }
//   B2ERROR("Unexpected EWirePosition");
//   return CDCGeometryPar::EWirePosition::c_Base;
// }

// EWirePosition fromCDC(CDCGeometryPar::EWirePosition wirePosSet)
// {
//   if (wirePosSet == CDCGeometryPar::EWirePosition::c_Base) {
//     return EWirePosition::c_Base;
//   } else if (wirePosSet == CDCGeometryPar::EWirePosition::c_Misaligned) {
//     return EWirePosition::c_Misaligned;
//   } else if (wirePosSet == CDCGeometryPar::EWirePosition::c_Aligned) {
//     return EWirePosition::c_Aligned;
//   }
//   B2ERROR("Unexpected EWirePosition");
//   return EWirePosition::c_Base;
// }
