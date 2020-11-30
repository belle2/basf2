/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty hearty@physics.ubc.ca                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDumpGeometry/ECLDumpGeometryModule.h>
#include <ecl/geometry/ECLGeometryPar.h>

using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDumpGeometry)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDumpGeometryModule::ECLDumpGeometryModule() : Module()
{
  // Set module properties
  setDescription("Print out location of every crystal");
  setPropertyFlags(c_ParallelProcessingCertified);
}


void ECLDumpGeometryModule::initialize()
{
}


void ECLDumpGeometryModule::event()
{
  if (firstEvent) {
    firstEvent = false;

    //..ECL geometry
    Belle2::ECL::ECLGeometryPar* eclGeometry = Belle2::ECL::ECLGeometryPar::Instance();
    std::printf("\nLocation and direction of the axis of each ECL crystal\n");
    std::printf("cellID     x          y          z    axisTheta axisPhi  \n");
    for (int cellID = 1; cellID <= 8736; cellID++) {
      TVector3 pos = eclGeometry->GetCrystalPos(cellID - 1);
      TVector3 dir = eclGeometry->GetCrystalVec(cellID - 1);
      std::printf("%6d %9.4f %9.4f %9.4f %9.6f %9.6f\n", cellID, pos.X(), pos.Y(), pos.Z(), dir.Theta(), dir.Phi());
    }
    std::printf("\n\n");
  }
}


