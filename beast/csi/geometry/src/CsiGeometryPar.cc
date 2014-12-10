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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <cmath>
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
  int cellID = 0;

  vector< string > partName;
  split(partName, VolumeName, is_any_of("_"));

  int iEnclosure = -1;
  int iCrystal   = -1;
  for (std::vector<string>::iterator it = partName.begin() ; it != partName.end(); ++it) {
    if (equals(*it, "Enclosure")) iEnclosure = boost::lexical_cast<int>(*(it + 1)) - 1;
    else if (equals(*it, "Crystal")) iCrystal = boost::lexical_cast<int>(*(it + 1)) - 1;
  }

  cellID = 2 * iEnclosure + iCrystal;

  if (cellID < 0) B2WARNING("CsiGeometryPar: volume " << VolumeName << " is not a crystal");

  return cellID;
}
