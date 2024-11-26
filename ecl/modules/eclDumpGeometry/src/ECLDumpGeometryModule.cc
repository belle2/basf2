/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDumpGeometry/ECLDumpGeometryModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* ROOT headers. */
#include <Math/Vector3D.h>

using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDumpGeometry);

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
    for (int cellID = 1; cellID <= ECLElementNumbers::c_NCrystals; cellID++) {
      ROOT::Math::XYZVector pos = eclGeometry->GetCrystalPos(cellID - 1);
      ROOT::Math::XYZVector dir = eclGeometry->GetCrystalVec(cellID - 1);
      std::printf("%6d %9.4f %9.4f %9.4f %9.6f %9.6f\n", cellID, pos.X(), pos.Y(), pos.Z(), dir.Theta(), dir.Phi());
    }
    std::printf("\n\n");
  }
}


