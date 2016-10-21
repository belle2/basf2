/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/VXDAlignmentPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>



using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
void VXDAlignmentPar::read(const GearDir& params)
{

  //B2INFO("Reading alignment data from PXD.xml ... ");

  //std::map<std::string, VXDAlignmentComponentPar > m_components;


  /*
  string path = (boost::format("Align[@component='%1%']/") % component).str();
      GearDir params(m_alignment, path);
      if (!params) {
        B2WARNING("Could not find alignment parameters for component " << component);
        return G4Transform3D();
      }
      double dU = params.getLength("du") / Unit::mm;
      double dV = params.getLength("dv") / Unit::mm;
      double dW = params.getLength("dw") / Unit::mm;
      double alpha = params.getAngle("alpha");
      double beta  = params.getAngle("beta");
      double gamma = params.getAngle("gamma");
      G4RotationMatrix rotation(alpha, beta, gamma);
      G4ThreeVector translation(dU, dV, dW);
      return G4Transform3D(rotation, translation);
  */

}





