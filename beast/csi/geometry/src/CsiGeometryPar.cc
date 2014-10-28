/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Alexandre Beaulieu                                       *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <cmath>
#include <boost/format.hpp>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;
using namespace csi;

#define PI 3.14159265358979323846


CsiGeometryPar* CsiGeometryPar::m_B4CsiGeometryParDB = 0;

CsiGeometryPar* CsiGeometryPar::Instance()
{
  if (!m_B4CsiGeometryParDB) m_B4CsiGeometryParDB = new CsiGeometryPar();
  return m_B4CsiGeometryParDB;
}

CsiGeometryPar::CsiGeometryPar()
{
  clear();
  read();
}

CsiGeometryPar::~CsiGeometryPar()
{
  if (m_B4CsiGeometryParDB) {
    delete m_B4CsiGeometryParDB;
    B2INFO("m_B4CsiGeometryParDB deleted ");
  }
}

void CsiGeometryPar::clear()
{
  /*
  mPar_cellID = 0;
  mPar_thetaID = 0;
  mPar_phiID = 0;
  mPar_thetaIndex = 0;
  mPar_phiIndex = 0;
  */

}

void CsiGeometryPar::read()
{
  /*
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"ECL\"]/Content/");
  for (int iCry = 1 ; iCry <= 132 ; ++iCry) {

    GearDir counter(content);
    counter.append((format("EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());

    m_ECThetaCrystal[iCry - 1]  = counter.getAngle("K_Ptheta") * 180 / PI;
    m_ECPhiCrystal[iCry - 1] = counter.getAngle("K_Rphi2")  * 180 / PI;
    m_ECRpos[iCry - 1]  = counter.getLength("K_Pr") ;
    m_ECThetapos[iCry - 1]  = counter.getAngle("K_Ptheta") * 180 / PI;
    m_ECPhipos[iCry - 1]  = counter.getAngle("K_Pphi") * 180 / PI;
  }
  */
}


int CsiGeometryPar::CsiVolNameToCellID(const G4String VolumeName)
{
  char temp1[100], temp2[100], temp3[100], temp4[100], temp5[100], temp6[100], temp7[100], temp8[100];
  int cellID = 0;
  sscanf(VolumeName.c_str(), "%100[^'_']_%100[^'_']_%100[^'_']_%100[^'_']_%100[^'_']_%100[^'_']_%100[^'_']_%100[^'_']", temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8);

  int GSector = atoi(temp4) - 1;
  int iCry = atoi(temp8);

  // This is the output of the geometry creator:
  /*
  [INFO] Volumes names for the CsI crystals
  [INFO]   Forward:
  [INFO]    Crystal Name av_1_impr_1_logicalEclFwCrystal_1_pv_0
  [INFO]    Crystal Name av_1_impr_1_logicalEclFwCrystal_2_pv_1
  [INFO]    Crystal Name av_1_impr_2_logicalEclFwCrystal_1_pv_0
  [INFO]    Crystal Name av_1_impr_2_logicalEclFwCrystal_2_pv_1
  [INFO]    Crystal Name av_1_impr_3_logicalEclFwCrystal_1_pv_0
  [INFO]    Crystal Name av_1_impr_3_logicalEclFwCrystal_2_pv_1
  [INFO]    Crystal Name av_1_impr_4_logicalEclFwCrystal_1_pv_0
  [INFO]    Crystal Name av_1_impr_4_logicalEclFwCrystal_2_pv_1
  [INFO]   Backward:
  [INFO]    Crystal Name av_4_impr_1_logicalEclBwCrystal_129_pv_0
  [INFO]    Crystal Name av_4_impr_2_logicalEclBwCrystal_129_pv_0
  [INFO]    Crystal Name av_4_impr_3_logicalEclBwCrystal_129_pv_0
  [INFO]    Crystal Name av_4_impr_4_logicalEclBwCrystal_129_pv_0

  */


  if (VolumeName.c_str() == 0 || GSector > 4) {
    B2WARNING("BEAST CsI CsiGeometryPar VolNameToCell; Sector  " << GSector << ". Out of range.");
    return -1;

  } else if (iCry > 200) {
    B2WARNING("BEAST CsI CsiGeometryPar VolNameToCell; Crystal " << iCry << ". Out of range.");
    return -1;

  } else {
    cellID = GSector * 4 + iCry;
  }
  return cellID;
}
